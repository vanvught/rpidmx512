/**
 * @file showfileprotocolnodeartnet.h
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_
#define PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_

#include <cstdint>
#include <cstring>

#include "artnetnode.h"
#include "artnet.h"
#include "firmware/debug/debug_debug.h"

class ShowFileProtocol
{
   public:
    ShowFileProtocol()
    {
        DEBUG_ENTRY();

        memcpy(dmx_.id, artnet::kNodeId, sizeof(dmx_.id));
        dmx_.op_code = static_cast<uint16_t>(artnet::OpCodes::kOpDmx);
        dmx_.prot_ver_hi = 0;
        dmx_.prot_ver_lo = artnet::kProtocolRevision;

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

        ArtNetNode::Get()->SetRecordShowfile(false);

        DEBUG_EXIT();
    }

    void Record()
    {
        DEBUG_ENTRY();

        ArtNetNode::Get()->SetRecordShowfile(true);

        DEBUG_EXIT();
    }

    void DmxOut(uint16_t universe, const uint8_t* data, uint32_t length)
    {
        memcpy(dmx_.data, data, length);

        if ((length & 0x1) == 0x1)
        {
            dmx_.data[length] = 0x00;
            length++;
        }

        dmx_.sequence = sequence_++;
        dmx_.physical = static_cast<uint8_t>(dmxnode::kMaxPorts + 1U);
        dmx_.port_address = universe;
        dmx_.length_hi = static_cast<uint8_t>((length & 0xFF00) >> 8);
        dmx_.length = static_cast<uint8_t>(length & 0xFF);

        ArtNetNode::Get()->HandleShowFile(&dmx_);
    }

    void DmxSync() {}

    void DmxBlackout() {}

    void DmxMaster([[maybe_unused]] uint32_t master) {}

    void DoRunCleanupProcess([[maybe_unused]] bool run) {}

    void Run() {}

    bool IsSyncDisabled() { return false; }

    void Print() {}

   private:
    artnet::ArtDmx dmx_;
    uint8_t sequence_{0};
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_ */
