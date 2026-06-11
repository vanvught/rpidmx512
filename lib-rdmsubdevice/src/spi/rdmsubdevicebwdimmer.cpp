/**
 * @file rdmsubdevicebwdimmer.cpp
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

#include "spi/rdmsubdevicebwdimmer.h"
#include "bwspidimmer.h"

static constexpr uint32_t kDmxFootprint = 1;
static RdmPersonality *s_rdm_personalities[] = {new RdmPersonality("Dimmer", kDmxFootprint)};

RDMSubDeviceBwDimmer::RDMSubDeviceBwDimmer(uint16_t dmx_start_address, char chip_select, uint8_t device_address, [[maybe_unused]] uint32_t spi_speed_hz) :
	RDMSubDevice("bw_spi_dimmer", dmx_start_address), spi_dimmer_(chip_select, device_address)
{
	SetDmxFootprint(kDmxFootprint);
	SetPersonalities(s_rdm_personalities, 1);
}

void RDMSubDeviceBwDimmer::Data(const uint8_t* data, uint32_t length) {
	const auto kDmxStartAddress = GetDmxStartAddress();

	if (kDmxStartAddress <= length) {
		const uint8_t kData = data[kDmxStartAddress - 1];
		if (data_ != kData) {
			spi_dimmer_.Output(kData);
			data_ = kData;
		}
	}
}

void RDMSubDeviceBwDimmer::UpdateEvent(TRDMSubDeviceUpdateEvent event) {
	if (event == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS) {
		Stop();
	}
}

