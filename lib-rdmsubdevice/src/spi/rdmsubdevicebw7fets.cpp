/**
 * @file rdmsubdevicebw7fets.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "spi/rdmsubdevicebw7fets.h"
#include "bwspi7fets.h"

static constexpr uint32_t kDmxFootprint = 7;
static RdmPersonality* s_rdm_personalities[] = {new RdmPersonality("Digital output 7-lines", kDmxFootprint)};

RDMSubDeviceBw7fets::RDMSubDeviceBw7fets(uint16_t dmx_start_address, char chip_select, uint8_t device_address, [[maybe_unused]] uint32_t spi_speed_hz)
    : RDMSubDevice("bw_spi_7fets", dmx_start_address), spi7fets_(chip_select, device_address) {
    SetDmxFootprint(kDmxFootprint);
    SetPersonalities(s_rdm_personalities, 1);
}

void RDMSubDeviceBw7fets::Data(const uint8_t* data, uint32_t length) {
    uint8_t d = 0;
    const uint32_t kDmxStartAddress = GetDmxStartAddress();

    for (uint32_t i = (kDmxStartAddress - 1), j = 0; (i < length) && (j < kDmxFootprint); i++, j++) {
        if ((data[i] & 0x80) != 0) { // 0-127 is off, 128-255 is on
            d = static_cast<uint8_t>(d | (1 << j));
        }
    }

    if (data_ != d) {
        spi7fets_.Output(d);
        data_ = d;
    }
}

void RDMSubDeviceBw7fets::UpdateEvent(TRDMSubDeviceUpdateEvent event) {
    if (event == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
        Stop();
    }
}
