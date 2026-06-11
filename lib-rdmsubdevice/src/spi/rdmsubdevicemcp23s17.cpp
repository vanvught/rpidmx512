/**
 * @file rdmsubdevicemcp23s17.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "spi/rdmsubdevicemcp23s17.h"
#include "mcp23s17.h"

static constexpr uint32_t kDmxFootprint = 16;
static RdmPersonality* s_rdm_personalities[] = {new RdmPersonality("Digital output 16-lines", kDmxFootprint)};

RDMSubDeviceMCP23S17::RDMSubDeviceMCP23S17(uint16_t dmx_start_address, char chip_select, uint8_t device_address, uint32_t spi_speed_hz) : RDMSubDevice("mcp23s17", dmx_start_address), m_MCP23S17(chip_select, spi_speed_hz, device_address) {
    SetDmxFootprint(kDmxFootprint);
    SetPersonalities(s_rdm_personalities, 1);
}

void RDMSubDeviceMCP23S17::Data(const uint8_t* data, uint32_t length) {
    uint16_t d = 0;
    const auto kDmxStartAddress = GetDmxStartAddress();

    for (uint32_t i = (kDmxStartAddress - 1), j = 0; (i < length) && (j < kDmxFootprint); i++, j++) {
        if ((data[i] & 0x80) != 0) { // 0-127 is off, 128-255 is on
            d = static_cast<uint8_t>(d | (1U << i));
        }
    }

    if (data_ != d) {
        m_MCP23S17.WriteRegister(mcp23x17::REG_GPIOA, d);
        data_ = d;
    }
}

void RDMSubDeviceMCP23S17::UpdateEvent(TRDMSubDeviceUpdateEvent event) {
    if (event == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
        Stop();
    }
}
