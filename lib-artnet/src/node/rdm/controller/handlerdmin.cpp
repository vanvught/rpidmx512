/**
 * @file handlerdmin.cpp
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-funroll-loops")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>

#include "artnetnode.h"
#include "artnet.h"
#include "artnetrdmcontroller.h"
#include "rdm.h"
#include "network.h"
#if defined(CONFIG_PANELLED_RDM_PORT) || defined(CONFIG_PANELLED_RDM_NO_PORT)
#include "hal_panelled.h"
#endif

void ArtNetNode::HandleRdmIn()
{
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        auto* art_rdm = &art_tod_packet_.art_rdm;

        if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
        {
            const auto* rdm_data = Rdm::Receive(port_index);
            if (rdm_data != nullptr)
            {
                if (rdm_controller_.RdmReceive(port_index, rdm_data))
                {
                    art_rdm->OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpRdm);
                    art_rdm->RdmVer = 0x01;
                    art_rdm->Net = node_.port[port_index].net_switch;
                    art_rdm->Command = 0;
                    art_rdm->Address = node_.port[port_index].sw;

                    auto* message = reinterpret_cast<const struct TRdmMessage*>(rdm_data);
                    memcpy(art_rdm->RdmPacket, &rdm_data[1], message->message_length + 1U);

                    const auto* rdm_message = reinterpret_cast<const struct TRdmMessageNoSc*>(art_rdm->RdmPacket);

                    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(art_rdm),
                                   ((sizeof(struct artnet::ArtRdm)) - 256) + rdm_message->message_length + 1, input_port_[port_index].destination_ip,
                                   artnet::kUdpPort);

#if defined(CONFIG_PANELLED_RDM_PORT)
                    hal::panelled::On(hal::panelled::PORT_A_RDM << port_index);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
                    hal::panelled::On(hal::panelled::RDM << port_index);
#endif
                }
            }
        }
        else if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            if (output_port_[port_index].rdm_destination_ip != 0) // && (!rdm_controller_.IsRunning(port_index)))
            {
                const auto* rdm_data = Rdm::Receive(port_index);
                if (rdm_data != nullptr)
                {
                    art_rdm->OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpRdm);
                    art_rdm->RdmVer = 0x01;
                    art_rdm->Net = node_.port[port_index].net_switch;
                    art_rdm->Command = 0;
                    art_rdm->Address = node_.port[port_index].sw;

                    auto* message = reinterpret_cast<const struct TRdmMessage*>(rdm_data);
                    memcpy(art_rdm->RdmPacket, &rdm_data[1], message->message_length + 1U);

                    const auto* rdm_message = reinterpret_cast<const struct TRdmMessageNoSc*>(art_rdm->RdmPacket);

                    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(art_rdm),
                                   ((sizeof(struct artnet::ArtRdm)) - 256) + rdm_message->message_length + 1, output_port_[port_index].rdm_destination_ip,
                                   artnet::kUdpPort);

                    output_port_[port_index].rdm_destination_ip = 0;

#if defined(CONFIG_PANELLED_RDM_PORT)
                    hal::panelled::On(hal::panelled::PORT_A_RDM << port_index);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
                    hal::panelled::On(hal::panelled::RDM << port_index);
#endif
                }
            }
        }
    }
}
