/**
 * @file rdmsubdevicemcp23s08.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "spi/rdmsubdevicemcp23s08.h"

#include "mcp23s08.h"

static constexpr uint32_t DMX_FOOTPRINT = 8;
static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Digital output 8-lines", DMX_FOOTPRINT)};

RDMSubDeviceMCP23S08::RDMSubDeviceMCP23S08(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("mcp23s08", nDmxStartAddress), m_MCP23S08(nChipSselect, nSpiSpeed, nSlaveAddress)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

void RDMSubDeviceMCP23S08::Data(const uint8_t* pData, uint32_t nLength) {
	uint8_t nData = 0;
	const uint32_t nDmxStartAddress = GetDmxStartAddress();

	for (uint32_t i = (nDmxStartAddress - 1), j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		if ((pData[i] & 0x80) != 0) {	// 0-127 is off, 128-255 is on
			nData = static_cast<uint8_t>(nData | (1U << j));
		}
	}

	if (m_nData != nData) {
		m_MCP23S08.WriteRegister(gpio::mcp23s08::reg::GPIO, nData);
		m_nData = nData;
	}
}

void RDMSubDeviceMCP23S08::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
