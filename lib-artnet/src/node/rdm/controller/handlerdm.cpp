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

#if defined(DEBUG_ARTNET_RDM)
#undef NDEBUG
#endif

#include <cstring>
#include <cassert>

#include "artnet.h"
#include "artnetnode.h"
#include "rdm.h"
#include "network.h"
#include "hal_panelled.h"
#include "rdm_discovery.h"
#include "firmware/debug/debug_debug.h"

namespace rdm::discovery
{
void Starting(uint32_t port_index, [[maybe_unused]] Type type)
{
	DEBUG_PRINTF("%u:%c", port_index, type == rdm::discovery::Type::kFull ? 'F' : 'I');
    
	auto& artnet = *ArtNetNode::Get();
    artnet.StopOutputPort(port_index);
}

void Finished(uint32_t port_index, [[maybe_unused]] Type type)
{
	DEBUG_PRINTF("%u:%c", port_index, type == rdm::discovery::Type::kFull ? 'F' : 'I');
	
    auto& artnet = *ArtNetNode::Get();
    artnet.SendArtTodData(port_index);
}
} // namespace rdm::discovery

/**
 * ArtTodControl is used to for an Output Gateway to flush its ToD and commence full discovery.
 * If the Output Gateway has physical DMX512 ports, discovery could take minutes.
 */
void ArtNetNode::HandleTodControl()
{
    DEBUG_ENTRY();

    const auto* const kArtTodControl = reinterpret_cast<artnet::ArtTodControl*>(receive_buffer_);

    if (kArtTodControl->command == artnet::TodControlCommand::kAtcNone)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>((kArtTodControl->net << 8)) | static_cast<uint16_t>((kArtTodControl->address));

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (kPortAddress != node_.port[port_index].port_address)
        {
            continue;
        }

        if ((node_.port[port_index].direction == dmxnode::PortDirection::kOutput) &&
            ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) != artnet::GoodOutputB::kRdmDisabled))
        {
            switch (kArtTodControl->command)
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
            if (kArtTodControl->command == artnet::TodControlCommand::kAtcFlush)
            {
                rdm_controller_.TodReset(port_index);
            }
        }
    }

    DEBUG_EXIT();
}

/**
 * All Input Gateways parse the ArtTodData packets.
 * If the Sub-Net and Universe fields match, the Input Gateway adds the TOD contents to their own internal TOD.
 * This allows Input Gateways to respond to any physical layer RDM discovery commands they receive.
 */
void ArtNetNode::HandleTodData()
{
    DEBUG_ENTRY();

    const auto* const kArtTodData = reinterpret_cast<const artnet::ArtTodData*>(receive_buffer_);

    if (kArtTodData->rdm_version != 0x01) [[unlikely]]
    {
        DEBUG_EXIT();
        return;
    }

    // Defensive: uid_count must never exceed array bound
    if (kArtTodData->uid_count > 200) [[unlikely]]
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>(static_cast<uint16_t>(kArtTodData->net) << 8) | static_cast<uint16_t>(kArtTodData->address);

    // Art-Net 4: bind_index discriminates packets from the same IP.
    // If bind_index is non-zero, treat it as selecting the logical port instance.
    if (kArtTodData->bind_index != 0)
    {
        const uint32_t kPortIndex = static_cast<uint32_t>(kArtTodData->bind_index - 1);

        // Validate bind_index mapping
        if (kPortIndex >= dmxnode::kMaxPorts)
        {
            DEBUG_EXIT();
            return;
        }

        // Only Input Gateways parse ArtTodData
        if (node_.port[kPortIndex].direction != dmxnode::PortDirection::kInput)
        {
            DEBUG_EXIT();
            return;
        }

        if (node_.port[kPortIndex].port_address != kPortAddress)
        {
            DEBUG_EXIT();
            return;
        }

        DEBUG_PRINTF("bind_index=%u -> kPortIndex=%u, kPortAddress=%u, uid_count=%u", kArtTodData->bind_index, kPortIndex, kPortAddress,
                     kArtTodData->uid_count);

        for (uint32_t uid_index = 0; uid_index < kArtTodData->uid_count; uid_index++)
        {
            const uint8_t* uid = kArtTodData->tod[uid_index];
            rdm_controller_.TodAddUid(kPortIndex, uid);
        }

        DEBUG_EXIT();
        return;
    }

    // Backward compatible / bind_index == 0 fallback:
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].direction != dmxnode::PortDirection::kInput)
        {
            continue;
        }

        if (node_.port[port_index].port_address == kPortAddress)
        {
            DEBUG_PRINTF("port_index=%u, kPortAddress=%u, uid_count=%u", port_index, kPortAddress, kArtTodData->uid_count);

            for (uint32_t uid_index = 0; uid_index < kArtTodData->uid_count; uid_index++)
            {
                const uint8_t* uid = kArtTodData->tod[uid_index];
                rdm_controller_.TodAddUid(port_index, uid);
            }
        }
    }

    DEBUG_EXIT();
}

/**
 * Node Output Gateway
 * ArtTodData packet is unicast to all IP addresses that have previously requested an ArtTodData packet.
 */
void ArtNetNode::SendArtTodData(uint32_t port_index)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("port_index=%u", port_index);
    assert(port_index < dmxnode::kMaxPorts);

    auto& tod_data = art_tod_packet_.art_tod_data;
    const auto kPage = port_index;

    memcpy(tod_data.id, artnet::kNodeId, sizeof(tod_data.id));
    tod_data.op_code = static_cast<uint16_t>(artnet::OpCodes::kOpToddata);
    tod_data.prot_ver_hi = 0;
    tod_data.prot_ver_lo = artnet::kProtocolRevision;
    tod_data.rdm_version = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.

    const auto kDiscovered = static_cast<uint8_t>(rdm_controller_.TodUidCount(port_index));

    // Physical Port = (BindIndex-1) * ArtPollReply->NumPortsLo + ArtTodData->Port
    // As most modern Art-Net gateways implement one universe per ArtPollReply,
    // ArtTodData->Port will usually be set to a value of 1.
    tod_data.port = static_cast<uint8_t>(1U + (port_index & 0x3));
    tod_data.spare1 = 0;
    tod_data.spare2 = 0;
    tod_data.spare3 = 0;
    tod_data.spare4 = 0;
    tod_data.spare5 = 0;
    tod_data.spare6 = 0;
    tod_data.bind_index = static_cast<uint8_t>(kPage + 1U); ///< ArtPollReplyData->BindIndex == ArtTodData ->BindIndex
    tod_data.net = node_.port[kPage].net_switch;
    tod_data.command_response = 0; ///< The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
    tod_data.address = node_.port[port_index].sw;
    tod_data.uid_total_hi = 0;
    tod_data.uid_total_lo = kDiscovered;
    tod_data.block_count = 0;
    tod_data.uid_count = kDiscovered;

    rdm_controller_.TodCopy(port_index, reinterpret_cast<uint8_t*>(tod_data.tod));

    const auto kLength = sizeof(struct artnet::ArtTodData) - (sizeof(tod_data.tod)) + (kDiscovered * 6U);

    for (auto& entry : state_.art.tod_request_ip_list)
    {
        if (entry != 0)
        {
            network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&tod_data), static_cast<uint16_t>(kLength), entry, artnet::kUdpPort);
        }
    }

    DEBUG_PRINTF("kDiscovered=%u", kDiscovered);
    DEBUG_EXIT();
}

/**
 * Node Input Gateway -> Input Gateway Directed Broadcasts to all nodes.
 */
void ArtNetNode::SendTodRequest(uint32_t port_index)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);

    rdm_controller_.TodReset(port_index);

    auto* tod_request = &art_tod_packet_.art_tod_request;
    const auto kPage = port_index;

    memcpy(tod_request->id, artnet::kNodeId, sizeof(tod_request->id));
    tod_request->op_code = static_cast<uint16_t>(artnet::OpCodes::kOpTodrequest);
    tod_request->prot_ver_hi = 0;
    tod_request->prot_ver_lo = artnet::kProtocolRevision;
    tod_request->spare1 = 0;
    tod_request->spare2 = 0;
    tod_request->spare3 = 0;
    tod_request->spare4 = 0;
    tod_request->spare5 = 0;
    tod_request->spare6 = 0;
    tod_request->spare7 = 0;
    tod_request->net = node_.port[kPage].net_switch;
    tod_request->command = 0;
    tod_request->add_count = 1;
    tod_request->address[0] = node_.port[port_index].sw;

    const auto kLength = sizeof(struct artnet::ArtTodRequest) - (sizeof(tod_request->address)) + tod_request->add_count;

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(tod_request), static_cast<uint16_t>(kLength), network::GetBroadcastIp(), artnet::kUdpPort);

    DEBUG_EXIT();
}

void ArtNetNode::HandleRdm()
{
    auto* const kArtRdm = reinterpret_cast<artnet::ArtRdm*>(receive_buffer_);

    if (kArtRdm->rdm_version != 0x01)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kPortAddress = static_cast<uint16_t>((kArtRdm->net << 8)) | static_cast<uint16_t>((kArtRdm->address));

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].port_address != kPortAddress)
        {
            continue;
        }

        if (rdm_controller_.IsRunning(port_index))
        {
            continue;
        }

        if ((node_.port[port_index].direction == dmxnode::PortDirection::kOutput) &&
            ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) != artnet::GoodOutputB::kRdmDisabled))
        {
#if (ARTNET_VERSION >= 4)
            if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
            {
                constexpr auto kMask = artnet::GoodOutput::kOutputIsMerging | artnet::GoodOutput::kDataIsBeingTransmitted | artnet::GoodOutput::kOutputIsSacn;
                output_port_[port_index].is_transmitting = (GetGoodOutput4(port_index) & kMask) != 0;
            }
#endif
            StopOutputPort(port_index);

            output_port_[port_index].rdm_destination_ip = ip_address_from_;

            auto* message = reinterpret_cast<const TRdmMessage*>(&kArtRdm->address);

            kArtRdm->address = E120_SC_RDM;
            Rdm::SendRaw(port_index, &kArtRdm->address, message->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
            rdm::message::Print(reinterpret_cast<const uint8_t*>(message));
#endif

#if defined(CONFIG_PANELLED_RDM_PORT)
            hal::panelled::On(hal::panelled::PORT_A_RDM << port_index);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
            hal::panelled::On(hal::panelled::RDM << port_index);
#endif
        }
        else if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            auto* rdm_message = reinterpret_cast<const TRdmMessage*>(&kArtRdm->address);

            if ((rdm_message->command_class == E120_GET_COMMAND_RESPONSE) || (rdm_message->command_class == E120_SET_COMMAND_RESPONSE))
            {
                kArtRdm->address = E120_SC_RDM;
                Rdm::SendRaw(port_index, reinterpret_cast<const uint8_t*>(rdm_message), rdm_message->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
                rdm::message::Print(reinterpret_cast<const uint8_t*>(rdm_message));
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
