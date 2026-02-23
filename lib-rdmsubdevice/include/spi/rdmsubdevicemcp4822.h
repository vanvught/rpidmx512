/**
 * @file rdmsubdevicemcp4822.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_RDMSUBDEVICEMCP4822_H_
#define SPI_RDMSUBDEVICEMCP4822_H_

#include <cstdint>

#include "rdmsubdevice.h"

#include "mcp48x2.h"

class RDMSubDeviceMCP4822 : public RDMSubDevice
{
   public:
    explicit RDMSubDeviceMCP4822(uint16_t nDmxStartAddress = 1, char nChipSselect = 0, uint8_t nSlaveAddress = 0, uint32_t nSpiSpeed = 0);

    bool Initialize() override { return true; }

    void Start() override {}

    void Stop() override
    {
        m_MCP4822.WriteDacAB(0, 0);
        m_nDataA = 0;
        m_nDataB = 0;
    }

    void Data(const uint8_t* pData, uint32_t nLength) override;

   private:
    void UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) override;

   private:
    dac::MCP4822 m_MCP4822;
    uint16_t m_nDataA = 0;
    uint16_t m_nDataB = 0;
};

#endif  // SPI_RDMSUBDEVICEMCP4822_H_
