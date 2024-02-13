/**
 * @file rdmsubdevicebwdio.cpp
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

#include "spi/rdmsubdevicebwdio.h"

#include "bwspidio.h"

static constexpr uint32_t DMX_FOOTPRINT = 7;
static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Digital output 7-lines", DMX_FOOTPRINT)};

RDMSubDeviceBwDio::RDMSubDeviceBwDio(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, [[maybe_unused]] uint32_t nSpiSpeed) :
	RDMSubDevice("bw_spi_dio", nDmxStartAddress), m_BwSpiDio(nChipSselect, nSlaveAddress)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

void RDMSubDeviceBwDio::Data(const uint8_t* pData, uint32_t nLength) {
	uint8_t nData = 0;
	const uint32_t nDmxStartAddress = GetDmxStartAddress();

	for (uint32_t i = (nDmxStartAddress - 1), j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		if ((pData[i] & 0x80) != 0) {	// 0-127 is off, 128-255 is on
			nData = static_cast<uint8_t>(nData | (1 << j));
		}
	}

	if (m_nData != nData) {
		m_BwSpiDio.Output(nData);
		m_nData = nData;
	}
}

void RDMSubDeviceBwDio::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
