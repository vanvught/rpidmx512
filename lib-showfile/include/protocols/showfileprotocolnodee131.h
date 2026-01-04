/**
 * @file showfileprotocolnodee131.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_
#define PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_

#include <cstdint>
#include <cstring>

#include "e131bridge.h"
#include "e131.h"
#include "e117.h"

#include "hal_uuid.h"

 #include "firmware/debug/debug_debug.h"

class ShowFileProtocol
{
   public:
    ShowFileProtocol()
    {
        DEBUG_ENTRY();
        // Root Layer (See Section 5)
        e131_data_packet_.root_layer.pre_amble_size = __builtin_bswap16(0x0010);
        e131_data_packet_.root_layer.post_amble_size = __builtin_bswap16(0x0000);
        memcpy(e131_data_packet_.root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
        e131_data_packet_.root_layer.vector = __builtin_bswap32(e131::vector::root::kData);
        hal::UuidCopy(e131_data_packet_.root_layer.cid);
        // E1.31 Framing Layer (See Section 6)
        e131_data_packet_.frame_layer.vector = __builtin_bswap32(e131::vector::data::kPacket);
        memcpy(e131_data_packet_.frame_layer.source_name, E131Bridge::Get()->GetSourceName(), e131::kSourceNameLength);
        e131_data_packet_.frame_layer.synchronization_address = __builtin_bswap16(0);
        e131_data_packet_.frame_layer.priority = e131::priority::kDefault;
        e131_data_packet_.frame_layer.options = 0;
        // Data Layer
        e131_data_packet_.dmp_layer.vector = e131::vector::dmp::kSetProperty;
        e131_data_packet_.dmp_layer.type = 0xa1;
        e131_data_packet_.dmp_layer.first_address_property = __builtin_bswap16(0x0000);
        e131_data_packet_.dmp_layer.address_increment = __builtin_bswap16(0x0001);
        e131_data_packet_.dmp_layer.property_values[0] = 0;

        DEBUG_EXIT();
    }

    void SetSynchronizationAddress([[maybe_unused]] uint16_t synchronization_address) {}

    void Start()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void Record()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void DmxOut(uint16_t universe, const uint8_t* dmx_data, uint32_t length)
    {
        length++; // Add 1 for SC
        // Root Layer (See Section 5)
        e131_data_packet_.root_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataRootLayerLength(length))));
        // E1.31 Framing Layer (See Section 6)
        e131_data_packet_.frame_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataFrameLayerLength(length))));
        e131_data_packet_.frame_layer.sequence_number = sequence_number_++;
        e131_data_packet_.frame_layer.universe = __builtin_bswap16(universe);
        // Data Layer
        e131_data_packet_.dmp_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DataLayerLength(length))));
        memcpy(e131_data_packet_.dmp_layer.property_values, &dmx_data[1], length - 1);
        e131_data_packet_.dmp_layer.property_value_count = __builtin_bswap16(static_cast<uint16_t>(length));

        E131Bridge::Get()->HandleShowFile(&e131_data_packet_);
    }

    void DmxSync() {}

    void DmxBlackout() {}

    void DmxMaster([[maybe_unused]] uint32_t master) {}

    void DoRunCleanupProcess([[maybe_unused]] bool do_run) {}

    void Run() {}

    bool IsSyncDisabled() { return false; }

    void Print() {}

   private:
    e131::DataPacket e131_data_packet_;
    uint8_t sequence_number_{0};
};

#endif  // PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_
