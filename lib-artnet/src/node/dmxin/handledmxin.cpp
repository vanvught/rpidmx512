/**
 * @file handledmxin.cpp
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#endif

#include <cstdint>
#include <cstring>

#include "artnetnode.h"
#include "artnet.h"
#include "dmx.h"
#include "network.h"
#include "hal_millis.h"
#include "hal.h"
#include "hal_panelled.h"
 #include "firmware/debug/debug_debug.h"

static uint32_t s_receiving_mask = 0;

void ArtNetNode::HandleDmxIn()
{
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if ((node_.port[port_index].direction == dmxnode::PortDirection::kInput) && (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet) &&
            ((input_port_[port_index].good_input & artnet::GoodInput::kDisabled) != artnet::GoodInput::kDisabled))
        {
            const auto* const kDataChanged = reinterpret_cast<const struct Data*>(Dmx::Get()->GetDmxChanged(port_index));

            if (kDataChanged != nullptr)
            {
                art_dmx_.Sequence = static_cast<uint8_t>(1U + input_port_[port_index].sequence_number++);
                art_dmx_.Physical = static_cast<uint8_t>(port_index);
                art_dmx_.PortAddress = node_.port[port_index].port_address;

                auto length = kDataChanged->Statistics.nSlotsInPacket;

                memcpy(art_dmx_.data, &kDataChanged->Data[1], length);

                if ((length & 0x1) == 0x1)
                {
                    art_dmx_.data[length] = 0x00;
                    length++;
                }

                art_dmx_.LengthHi = static_cast<uint8_t>((length & 0xFF00) >> 8);
                art_dmx_.Length = static_cast<uint8_t>(length & 0xFF);

                input_port_[port_index].good_input |= artnet::GoodInput::kDataRecieved;

                const auto* udp_data = reinterpret_cast<const uint8_t*>(&art_dmx_);
                network::udp::Send(handle_, udp_data, sizeof(struct artnet::ArtDmx), input_port_[port_index].destination_ip, artnet::kUdpPort);

                SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX sent", port_index);

                if (node_.port[port_index].local_merge)
                {
                    receive_buffer_ = reinterpret_cast<uint8_t*>(&art_dmx_);
                    ip_address_from_ = network::kIpaddrLoopback;
                    HandleDmx();

                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX local merge", port_index);
                }

                if ((s_receiving_mask & (1U << port_index)) != (1U << port_index))
                {
                    s_receiving_mask |= (1U << port_index);
                    state_.receiving_dmx |= (1U << static_cast<uint8_t>(dmxnode::PortDirection::kInput));
                    hal::panelled::On(hal::panelled::PORT_A_RX << port_index);
                }

                continue;
            }

            if (Dmx::Get()->GetDmxUpdatesPerSecond(port_index) == 0)
            {
                auto send_art_dmx = false;

                if ((input_port_[port_index].good_input & artnet::GoodInput::kDataRecieved) == artnet::GoodInput::kDataRecieved)
                {
                    input_port_[port_index].good_input = static_cast<uint8_t>(input_port_[port_index].good_input & ~artnet::GoodInput::kDataRecieved);
                    input_port_[port_index].millis = hal::Millis();
                    send_art_dmx = true;

                    s_receiving_mask &= ~(1U << port_index);
                    hal::panelled::Off(hal::panelled::PORT_A_RX << port_index);

                    if (s_receiving_mask == 0)
                    {
                        state_.receiving_dmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(dmxnode::PortDirection::kInput)));
                    }

                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX updates per second is 0", port_index);
                }
                else if (input_port_[port_index].millis != 0)
                {
                    const auto kMillis = hal::Millis();
                    if ((kMillis - input_port_[port_index].millis) > 1000)
                    {
                        input_port_[port_index].millis = kMillis;
                        send_art_dmx = true;

                        SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX timeout 1 second", port_index);
                    }
                }

                if (send_art_dmx)
                {
                    const auto* const kDataCurrent = reinterpret_cast<const struct Data*>(Dmx::Get()->GetDmxCurrentData(port_index));

                    art_dmx_.Sequence = static_cast<uint8_t>(1U + input_port_[port_index].sequence_number++);
                    art_dmx_.Physical = static_cast<uint8_t>(port_index);
                    art_dmx_.PortAddress = node_.port[port_index].port_address;

                    auto length = kDataCurrent->Statistics.nSlotsInPacket;

                    memcpy(art_dmx_.data, &kDataCurrent->Data[1], length);

                    if ((length & 0x1) == 0x1)
                    {
                        art_dmx_.data[length] = 0x00;
                        length++;
                    }

                    art_dmx_.LengthHi = static_cast<uint8_t>((length & 0xFF00) >> 8);
                    art_dmx_.Length = static_cast<uint8_t>(length & 0xFF);

                    const auto* udp_data = reinterpret_cast<const uint8_t*>(&art_dmx_);
                    network::udp::Send(handle_, udp_data, sizeof(struct artnet::ArtDmx), input_port_[port_index].destination_ip, artnet::kUdpPort);

                    SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX sent (timeout)", port_index);

                    if (node_.port[port_index].local_merge)
                    {
                        receive_buffer_ = reinterpret_cast<uint8_t*>(&art_dmx_);
                        ip_address_from_ = network::kIpaddrLoopback;
                        HandleDmx();

                        SendDiag(artnet::PriorityCodes::kDiagLow, "%u: Input DMX local merge", port_index);
                    }
                }
            }
        }
    }
}
