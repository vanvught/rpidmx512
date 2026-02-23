/**
 * @file handlerdm.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstring>
#include <cassert>

#include "artnet.h"
#include "artnetnode.h"
#include "rdm.h"
#include "network.h"
#include "hal_panelled.h"
 #include "firmware/debug/debug_debug.h"

namespace artnet::rdm::controller
{
void DiscoveryStart(uint32_t port_index)
{
    auto& artnet = *ArtNetNode::Get();
    artnet.GoodOutputBClear(port_index, artnet::GoodOutputB::kDiscoveryNotRunning);
}

void DiscoveryDone(uint32_t port_index)
{
    auto& artnet = *ArtNetNode::Get();
    artnet.GoodOutputBClear(port_index, artnet::GoodOutputB::kDiscoveryNotRunning);
    artnet.SendTod(port_index);
    artnet.RestartOutputPort(port_index);
}
} // namespace artnet::rdm::controller

/**
 * ArtTodControl is used to for an Output Gateway to flush its ToD and commence full discovery.
 * If the Output Gateway has physical DMX512 ports, discovery could take minutes.
 */
void ArtNetNode::HandleTodControl()
{
    DEBUG_ENTRY();

    const auto* const kArtTodControl = reinterpret_cast<artnet::ArtTodControl*>(receive_buffer_);

    if (kArtTodControl->Command == artnet::TodControlCommand::kAtcNone)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>((kArtTodControl->Net << 8)) | static_cast<uint16_t>((kArtTodControl->Address));

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (kPortAddress != node_.port[port_index].port_address)
        {
            continue;
        }

        if ((node_.port[port_index].direction == dmxnode::PortDirection::kOutput) &&
            ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) != artnet::GoodOutputB::kRdmDisabled))
        {
            switch (kArtTodControl->Command)
            {
                case artnet::TodControlCommand::kAtcFlush:
                    rdm_controller_.Full(port_index);
                    break;
                case artnet::TodControlCommand::kAtcEnd:
                    rdm_controller_.Stop(port_index);
                    break;
                case artnet::TodControlCommand::kAtcIncon:
                    rdm_controller_.EnableBackground(port_index);
                    output_port_[port_index].good_output_b &= static_cast<uint8_t>(~artnet::GoodOutputB::kDiscoveryDisabled);
                    break;
                case artnet::TodControlCommand::kAtcIncoff:
                    rdm_controller_.DisableBackground(port_index);
                    output_port_[port_index].good_output_b |= artnet::GoodOutputB::kDiscoveryDisabled;
                    break;
                default:
                    break;
            }
        }
        else if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            if (kArtTodControl->Command == artnet::TodControlCommand::kAtcFlush)
            {
                rdm_controller_.TodReset(port_index);
            }
        }
    }

    DEBUG_EXIT();
}

void ArtNetNode::HandleTodData()
{
    DEBUG_ENTRY();

    const auto* const kArtTodData = reinterpret_cast<artnet::ArtTodData*>(receive_buffer_);

    if (kArtTodData->RdmVer != 0x01)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>((kArtTodData->Net << 8)) | static_cast<uint16_t>((kArtTodData->Address));

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].direction != dmxnode::PortDirection::kInput)
        {
            continue;
        }

        if (node_.port[port_index].port_address == kPortAddress)
        {
            DEBUG_PRINTF("port_index=%u, kPortAddress=%u, pArtTodData->UidCount=%u", port_index, kPortAddress, kArtTodData->UidCount);

            for (uint32_t uid_index = 0; uid_index < kArtTodData->UidCount; uid_index++)
            {
                const uint8_t* uid = kArtTodData->Tod[uid_index];
                rdm_controller_.TodAddUid(port_index, uid);
            }
        }
    }

    DEBUG_EXIT();
}

/**
 * Output Gateway always Directed Broadcasts this packet.
 */
void ArtNetNode::SendTod(uint32_t port_index)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u", port_index);
    assert(port_index < dmxnode::kMaxPorts);

    auto& tod_data = art_tod_packet_.art_tod_data;
    const auto kPage = port_index;

    memcpy(tod_data.Id, artnet::kNodeId, sizeof(tod_data.Id));
    tod_data.OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpToddata);
    tod_data.ProtVerHi = 0;
    tod_data.ProtVerLo = artnet::kProtocolRevision;
    tod_data.RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.

    const auto kDiscovered = static_cast<uint8_t>(rdm_controller_.GetUidCount(port_index));

    /**
     * Physical Port = (BindIndex-1) * ArtPollReply- >NumPortsLo + ArtTodData->Port
     * As most modern Art-Net gateways implement one universe per ArtPollReply,
     * ArtTodData->Port will usually be set to a value of 1.
     */
    tod_data.Port = static_cast<uint8_t>(1U + (port_index & 0x3));
    tod_data.spare1 = 0;
    tod_data.spare2 = 0;
    tod_data.spare3 = 0;
    tod_data.spare4 = 0;
    tod_data.spare5 = 0;
    tod_data.spare6 = 0;
    tod_data.bind_index = static_cast<uint8_t>(kPage + 1U); ///< ArtPollReplyData->BindIndex == ArtTodData- >BindIndex
    tod_data.Net = node_.port[kPage].net_switch;
    tod_data.CommandResponse = 0; ///< The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
    tod_data.Address = node_.port[port_index].sw;
    tod_data.UidTotalHi = 0;
    tod_data.UidTotalLo = kDiscovered;
    tod_data.BlockCount = 0;
    tod_data.UidCount = kDiscovered;

    rdm_controller_.TodCopy(port_index, reinterpret_cast<uint8_t*>(tod_data.Tod));

    const auto kLength = sizeof(struct artnet::ArtTodData) - (sizeof(tod_data.Tod)) + (kDiscovered * 6U);

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&tod_data), static_cast<uint16_t>(kLength), network::GetBroadcastIp(), artnet::kUdpPort);

    DEBUG_PRINTF("kDiscovered=%u", kDiscovered);
    DEBUG_EXIT();
}

void ArtNetNode::SendTodRequest(uint32_t port_index)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);

    rdm_controller_.TodReset(port_index);

    auto* request = &art_tod_packet_.art_tod_request;
    const auto kPage = port_index;

    memcpy(request->Id, artnet::kNodeId, sizeof(request->Id));
    request->OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpTodrequest);
    request->ProtVerHi = 0;
    request->ProtVerLo = artnet::kProtocolRevision;
    request->spare1 = 0;
    request->spare2 = 0;
    request->spare3 = 0;
    request->spare4 = 0;
    request->spare5 = 0;
    request->spare6 = 0;
    request->spare7 = 0;
    request->Net = node_.port[kPage].net_switch;
    request->Command = 0;
    request->AddCount = 1;
    request->Address[0] = node_.port[port_index].sw;

    const auto kLength = sizeof(struct artnet::ArtTodRequest) - (sizeof(request->Address)) + request->AddCount;

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(request), static_cast<uint16_t>(kLength), network::GetBroadcastIp(), artnet::kUdpPort);

    DEBUG_EXIT();
}

void ArtNetNode::HandleRdm()
{
    auto* const kArtRdm = reinterpret_cast<artnet::ArtRdm*>(receive_buffer_);

    if (kArtRdm->RdmVer != 0x01)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>((kArtRdm->Net << 8)) | static_cast<uint16_t>((kArtRdm->Address));

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].port_address != kPortAddress)
        {
            continue;
        }

        if ((node_.port[port_index].direction == dmxnode::PortDirection::kOutput) &&
            ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) != artnet::GoodOutputB::kRdmDisabled))
        {
#if (ARTNET_VERSION >= 4)
            if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
            {
                constexpr auto kMask =
                    artnet::GoodOutput::kOutputIsMerging | artnet::GoodOutput::kDataIsBeingTransmitted | artnet::GoodOutput::kOutputIsSacn;
                output_port_[port_index].is_transmitting = (GetGoodOutput4(port_index) & kMask) != 0;
            }
#endif
            if (output_port_[port_index].is_transmitting)
            {
                output_port_[port_index].is_transmitting = false;
                dmxnode_output_type_->Stop(port_index); // Stop DMX if was running
            }

            output_port_[port_index].rdm_destination_ip = ip_address_from_;

            auto* message = reinterpret_cast<const TRdmMessage*>(&kArtRdm->Address);

            kArtRdm->Address = E120_SC_RDM;
            Rdm::SendRaw(port_index, &kArtRdm->Address, message->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
            rdm::MessagePrint(reinterpret_cast<const uint8_t*>(message));
#endif

#if defined(CONFIG_PANELLED_RDM_PORT)
            hal::panelled::On(hal::panelled::PORT_A_RDM << port_index);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
            hal::panelled::On(hal::panelled::RDM << port_index);
#endif
        }
        else if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            auto* rdm_message = reinterpret_cast<const TRdmMessage*>(&kArtRdm->Address);

            if ((rdm_message->command_class == E120_GET_COMMAND_RESPONSE) || (rdm_message->command_class == E120_SET_COMMAND_RESPONSE))
            {
                kArtRdm->Address = E120_SC_RDM;
                Rdm::SendRaw(port_index, reinterpret_cast<const uint8_t*>(rdm_message), rdm_message->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
                rdm::MessagePrint(reinterpret_cast<const uint8_t*>(rdm_message));
#endif
            }

#if defined(CONFIG_PANELLED_RDM_PORT)
            hal::panelled::On(hal::panelled::PORT_A_RDM << port_index);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
            hal::panelled::On(hal::panelled::RDM << port_index);
#endif
        }
    }
}
