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

#include <cstdint>
#include <cstring>

#include "e131.h"
#include "e131bridge.h"
#include "e117.h"
#include "dmx.h"
#include "network.h"

void E131Bridge::FillDataPacket()
{
    // Root Layer (See Section 5)
    e131_data_packet_.root_layer.pre_amble_size = __builtin_bswap16(0x0010);
    e131_data_packet_.root_layer.post_amble_size = __builtin_bswap16(0x0000);
    memcpy(e131_data_packet_.root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
    e131_data_packet_.root_layer.vector = __builtin_bswap32(e131::vector::root::kData);
    memcpy(e131_data_packet_.root_layer.cid, cid_, e117::kCidLength);
    // E1.31 Framing Layer (See Section 6)
    e131_data_packet_.frame_layer.vector = __builtin_bswap32(e131::vector::data::kPacket);
    memcpy(e131_data_packet_.frame_layer.source_name, source_name_, e131::kSourceNameLength);
    e131_data_packet_.frame_layer.synchronization_address = __builtin_bswap16(0); // Currently not supported
    e131_data_packet_.frame_layer.options = 0;
    // Data DMP Layer
    e131_data_packet_.dmp_layer.vector = e131::vector::dmp::kSetProperty;
    e131_data_packet_.dmp_layer.type = 0xa1;
    e131_data_packet_.dmp_layer.first_address_property = __builtin_bswap16(0x0000);
    e131_data_packet_.dmp_layer.address_increment = __builtin_bswap16(0x0001);
}

static uint32_t s_receiving_mask = 0;

void E131Bridge::HandleDmxIn()
{
    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if ((bridge_.port[port_index].direction == dmxnode::PortDirection::kInput) && (!input_port_[port_index].is_disabled))
        {
            const auto* const kDataChanged = reinterpret_cast<const struct Data*>(Dmx::Get()->GetDmxChanged(port_index));

            if (kDataChanged != nullptr)
            {
                // Root Layer (See Section 5)
                const auto kLength = (1U + kDataChanged->Statistics.nSlotsInPacket); // Add 1 for SC
                e131_data_packet_.root_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataRootLayerLength(kLength))));
                // E1.31 Framing Layer (See Section 6)
                e131_data_packet_.frame_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataFrameLayerLength(kLength))));
                e131_data_packet_.frame_layer.priority = input_port_[port_index].priority;
                e131_data_packet_.frame_layer.sequence_number = input_port_[port_index].sequence_number++;
                e131_data_packet_.frame_layer.universe = __builtin_bswap16(bridge_.port[port_index].universe);
                // Data Layer
                e131_data_packet_.dmp_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataLayerLength(kLength))));
                memcpy(e131_data_packet_.dmp_layer.property_values, kDataChanged, kLength);
                e131_data_packet_.dmp_layer.property_value_count = __builtin_bswap16(static_cast<uint16_t>(kLength));

                network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&e131_data_packet_), e131::DataPacketSize(kLength), input_port_[port_index].multicast_ip, e131::kUdpPort);

                if (bridge_.port[port_index].local_merge)
                {
                    receive_buffer_ = reinterpret_cast<uint8_t*>(&e131_data_packet_);
                    ip_address_from_ = network::kIpaddrLoopback;
                    HandleDmx();
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
                auto senddmx = false;

                if ((s_receiving_mask & (1U << port_index)) == (1U << port_index))
                {
                    senddmx = true;

                    s_receiving_mask &= ~(1U << port_index);
                    hal::panelled::Off(hal::panelled::PORT_A_RX << port_index);

                    if (s_receiving_mask == 0)
                    {
                        state_.receiving_dmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(dmxnode::PortDirection::kInput)));
                    }
                }
                else if (input_port_[port_index].millis != 0)
                {
                    const auto kMillis = hal::Millis();
                    if ((kMillis - input_port_[port_index].millis) > 1000)
                    {
                        input_port_[port_index].millis = kMillis;
                        senddmx = true;
                    }
                }

                if (senddmx)
                {
                    const auto* const kDataCurrent = reinterpret_cast<const struct Data*>(Dmx::Get()->GetDmxCurrentData(port_index));
                    // Root Layer (See Section 5)
                    const auto kLength = (1U + kDataCurrent->Statistics.nSlotsInPacket); // Add 1 for SC
                    e131_data_packet_.root_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataRootLayerLength(kLength))));
                    // E1.31 Framing Layer (See Section 6)
                    e131_data_packet_.frame_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataFrameLayerLength(kLength))));
                    e131_data_packet_.frame_layer.priority = input_port_[port_index].priority;
                    e131_data_packet_.frame_layer.sequence_number = input_port_[port_index].sequence_number++;
                    e131_data_packet_.frame_layer.universe = __builtin_bswap16(bridge_.port[port_index].universe);
                    // Data Layer
                    e131_data_packet_.dmp_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataLayerLength(kLength))));
                    memcpy(e131_data_packet_.dmp_layer.property_values, kDataCurrent, kLength);
                    e131_data_packet_.dmp_layer.property_value_count = __builtin_bswap16(static_cast<uint16_t>(kLength));

                    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&e131_data_packet_), e131::DataPacketSize(kLength), input_port_[port_index].multicast_ip, e131::kUdpPort);

                    if (bridge_.port[port_index].local_merge)
                    {
                        receive_buffer_ = reinterpret_cast<uint8_t*>(&e131_data_packet_);
                        ip_address_from_ = network::kIpaddrLoopback;
                        HandleDmx();
                    }
                }
            }
        }
    }
}
