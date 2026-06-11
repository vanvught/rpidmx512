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

#include "spi/rdmsubdevicebwdio.h"
#include "bwspidio.h"

static constexpr uint32_t DMX_FOOTPRINT = 7;
static RdmPersonality *s_RDMPersonalities[] = {new RdmPersonality("Digital output 7-lines", DMX_FOOTPRINT)};

RDMSubDeviceBwDio::RDMSubDeviceBwDio(uint16_t dmx_start_address, char chip_select, uint8_t device_address, [[maybe_unused]] uint32_t spi_speed_hz) :
	RDMSubDevice("bw_spi_dio", dmx_start_address), m_BwSpiDio(chip_select, device_address)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 1);
}

void RDMSubDeviceBwDio::Data(const uint8_t* data, uint32_t length) {
	uint8_t nData = 0;
	const auto dmx_start_address = GetDmxStartAddress();

	for (uint32_t i = (dmx_start_address - 1), j = 0; (i < length) && (j < DMX_FOOTPRINT); i++, j++) {
		if ((data[i] & 0x80) != 0) {	// 0-127 is off, 128-255 is on
			nData = static_cast<uint8_t>(nData | (1 << j));
		}
	}

	if (data_ != nData) {
		m_BwSpiDio.Output(nData);
		data_ = nData;
	}
}

void RDMSubDeviceBwDio::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	if (tUpdateEvent == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}
