/**
 * @file rdmsubdevicemcp4902.cpp
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
#include <cassert>

/*
 * MCP4902: Dual 8-Bit Voltage Output DAC
 */

#include "spi/rdmsubdevicemcp4902.h"
#include "mcp49x2.h"

static constexpr uint32_t kDmxFootprint = 2;
static RdmPersonality* s_rdm_personalities[] = {new RdmPersonality("Analog output 2-lines", kDmxFootprint)};

RDMSubDeviceMCP4902::RDMSubDeviceMCP4902(uint16_t dmx_start_address, char chip_select, [[maybe_unused]] uint8_t device_address, uint32_t spi_speed_hz) : RDMSubDevice("mcp4902", dmx_start_address), m_MCP4902(chip_select, spi_speed_hz) {
    SetDmxFootprint(kDmxFootprint);
    SetPersonalities(s_rdm_personalities, 1);
}

void RDMSubDeviceMCP4902::Data(const uint8_t* data, uint32_t length) {
    assert(nLength <= 512);

    auto offset = static_cast<uint16_t>(GetDmxStartAddress() - 1);

    if (offset < length) {
        const uint8_t kDataA = data[offset];

        if (kDataA != m_nDataA) {
            m_MCP4902.WriteDacA(kDataA);
            m_nDataA = kDataA;
        }

        offset++;

        if (offset < length) {
            const uint8_t kDataB = data[offset];

            if (kDataB != m_nDataB) {
                m_MCP4902.WriteDacB(kDataB);
                m_nDataB = kDataB;
            }
        }
    }
}

void RDMSubDeviceMCP4902::UpdateEvent(TRDMSubDeviceUpdateEvent event) {
    if (event == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
        Stop();
    }
}
