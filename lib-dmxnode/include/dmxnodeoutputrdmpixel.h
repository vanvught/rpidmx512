/**
 * @file dmxnodeoutputrdmpixel.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODEOUTPUTRDMPIXEL_H_
#define DMXNODEOUTPUTRDMPIXEL_H_

#include <cstdint>

#include "dmxnode.h"

class DmxNodeOutputRdmPixel
{
   public:
    DmxNodeOutputRdmPixel() = default;
    virtual ~DmxNodeOutputRdmPixel() = default;

    virtual void Start(uint32_t port_index) = 0;
    virtual void Stop(uint32_t port_index) = 0;

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length) { SetDataImpl(port_index, data, length, doUpdate); }

    virtual bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) { return false; }

    virtual uint16_t GetDmxStartAddress() { return dmxnode::kStartAddressDefault; }

    virtual uint16_t GetDmxFootprint() { return dmxnode::kUniverseSize; }

    virtual bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        slot_info.type = 0x00;       // ST_PRIMARY
        slot_info.category = 0x0001; // SD_INTENSITY
        return true;
    }

    virtual void Print() {}

   protected:
    virtual void SetDataImpl(uint32_t port_index, const uint8_t* data, uint32_t length, bool do_update) = 0;
};

#endif  // DMXNODEOUTPUTRDMPIXEL_H_
