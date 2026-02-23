/**
 * @file handlerdm.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include "artnetnode.h"
#include "artnetrdmresponder.h"
 #include "firmware/debug/debug_debug.h"

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
        if ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled)
        {
            continue;
        }

        if ((kPortAddress == node_.port[port_index].port_address) && (node_.port[port_index].direction == dmxnode::PortDirection::kOutput))
        {
            const auto* response = const_cast<uint8_t*>(rdm_responder_->Handler(port_index, kArtRdm->RdmPacket));

            if (response != nullptr)
            {
                kArtRdm->RdmVer = 0x01;

                const auto kMessageLength = static_cast<uint16_t>(response[2] + 1);
                memcpy(kArtRdm->RdmPacket, &response[1], kMessageLength);

                const auto kLength = sizeof(struct artnet::ArtRdm) - sizeof(kArtRdm->RdmPacket) + kMessageLength;

                network::udp::Send(handle_, receive_buffer_, kLength, ip_address_from_, artnet::kUdpPort);
            }
            else
            {
                DEBUG_PUTS("No RDM response");
            }
        }
    }
}

void ArtNetNode::HandleTodControl()
{
    DEBUG_ENTRY();

    const auto* const kArtTodControl = reinterpret_cast<artnet::ArtTodControl*>(receive_buffer_);
    const auto kPortAddress = static_cast<uint16_t>((kArtTodControl->Net << 8)) | static_cast<uint16_t>((kArtTodControl->Address));

    const uint32_t kPortIndex = 0;

    if ((output_port_[kPortIndex].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled)
    {
        DEBUG_EXIT();
        return;
    }

    if ((kPortAddress == node_.port[kPortIndex].port_address) && (node_.port[kPortIndex].direction == dmxnode::PortDirection::kOutput))
    {
        if (kArtTodControl->Command == 0x01)
        { // AtcFlush
            SendTod(kPortIndex);
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

    const auto kDiscovered = static_cast<uint8_t>(port_index == 0 ? 1 : 0);

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

    rdm_responder_->TodCopy(port_index, reinterpret_cast<uint8_t*>(tod_data.Tod));

    const auto kLength = sizeof(struct artnet::ArtTodData) - (sizeof(tod_data.Tod)) + (kDiscovered * 6U);

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&tod_data), kLength, network::GetBroadcastIp(), artnet::kUdpPort);

    DEBUG_EXIT();
}

void ArtNetNode::HandleTodData()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}
