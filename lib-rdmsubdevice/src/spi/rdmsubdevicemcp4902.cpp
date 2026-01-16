/**
 * @file rdmsubdevicemcp4902.cpp
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
#include <cassert>

/*
 * MCP4902: Dual 8-Bit Voltage Output DAC
 */

#include "spi/rdmsubdevicemcp4902.h"

#include "mcp49x2.h"

static constexpr uint32_t DMX_FOOTPRINT = 2;
static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Analog output 2-lines", DMX_FOOTPRINT)};

RDMSubDeviceMCP4902::RDMSubDeviceMCP4902(uint16_t nDmxStartAddress, char nChipSselect, [[maybe_unused]] uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("mcp4902",nDmxStartAddress), m_MCP4902(nChipSselect, nSpiSpeed)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

void RDMSubDeviceMCP4902::Data(const uint8_t* pData, uint32_t nLength) {
	assert(nLength <= 512);

	auto nOffset = static_cast<uint16_t>(GetDmxStartAddress() - 1);

	if (nOffset < nLength) {
		const uint8_t nDataA = pData[nOffset];

		if (nDataA != m_nDataA) {
			m_MCP4902.WriteDacA(nDataA);
			m_nDataA = nDataA;
		}

		nOffset++;

		if (nOffset < nLength) {
			const uint8_t nDataB = pData[nOffset];

			if (nDataB != m_nDataB) {
				m_MCP4902.WriteDacB(nDataB);
				m_nDataB = nDataB;
			}
		}
	}
}

void RDMSubDeviceMCP4902::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
