/**
 * @file l6470dmxmodes.h
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

#ifndef L6470DMXMODES_H_
#define L6470DMXMODES_H_

#include <cstdint>

#include "l6470.h"
#include "l6470dmxmode.h"

class L6470DmxModes
{
   public:
    L6470DmxModes(TL6470DmxModes dmxmode, uint16_t dmx_start_address, L6470*, uint32_t motor_index);
    ~L6470DmxModes();

    void InitSwitch();
    void InitPos();

    void HandleBusy();
    bool BusyCheck();

    bool IsDmxDataChanged(const uint8_t* dmx_data, uint32_t length);
    void DmxData(const uint8_t* dmx_data, uint32_t length);

    void Start();
    void Stop();

    void Print();

    TL6470DmxModes GetMode() { return dmxmode_; }

   public: // RDM
    void SetDmxStartAddress(uint16_t dmx_start_address) { dmx_start_address_ = dmx_start_address; }
    uint16_t GetDmxStartAddress() { return dmx_start_address_; }

    uint16_t GetDmxFootPrint() { return dmx_footprint_; }

   public:
    static uint16_t GetDmxFootPrintMode(uint32_t);

   private:
    bool IsDmxDataChanged(const uint8_t*);

   private:
    bool started_{false};

   private:
    uint8_t motor_number_{0};
    TL6470DmxModes dmxmode_{L6470DMXMODE_UNDEFINED};
    uint16_t dmx_start_address_;
    L6470DmxMode* l6470_dmxmode_{nullptr};
    uint16_t dmx_footprint_{0};
    uint8_t* dmx_data_;
};

#endif  // L6470DMXMODES_H_
