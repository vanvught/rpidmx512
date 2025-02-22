/**
 * @file rdmsubdevicedummy.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "subdevice/rdmsubdevicedummy.h"
#include "rdmpersonality.h"

static constexpr uint32_t DMX_FOOTPRINT = 4;
static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Dummy-H", DMX_FOOTPRINT), new RDMPersonality("Dummy-D", DMX_FOOTPRINT)};

RDMSubDeviceDummy::RDMSubDeviceDummy([[maybe_unused]] uint16_t nDmxStartAddress, [[maybe_unused]] char nChipSselect, [[maybe_unused]] uint8_t nSlaveAddress, [[maybe_unused]] uint32_t nSpiSpeed) :
	RDMSubDevice("SubDevice Dummy", nDmxStartAddress)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 2);
}

bool RDMSubDeviceDummy::Initialize() {
	return true;
}

void RDMSubDeviceDummy::Start() {
	printf("RDMSubDeviceDummy::Start()\n");
}

void RDMSubDeviceDummy::Stop() {
	printf("RDMSubDeviceDummy::Stop()\n");
}

void RDMSubDeviceDummy::Data(const uint8_t* pData, uint32_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= 512);

	const auto nDmxStartAddress = GetDmxStartAddress();

	printf("RDMSubDeviceDummy::Data(*pData:%p, nLength:%u)\n", static_cast<const void *>(pData), static_cast<unsigned int>(nLength));
	printf("%u:%u:%u: ", static_cast<unsigned int>(nLength), static_cast<unsigned int>(DMX_FOOTPRINT), static_cast<unsigned int>(nDmxStartAddress));

	for (uint32_t i = static_cast<uint32_t>(nDmxStartAddress - 1), j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		switch (GetPersonalityCurrent()) {
		case 1:
			printf("%.2x ", pData[i]);
			break;
		case 2:
			printf("%.3d ", static_cast<int>(pData[i]));
			break;
		default:
			break;
		}
	}

	puts("");
}

void RDMSubDeviceDummy::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	printf("RDMSubDeviceDummy::UpdateEvent(TRDMSubDeviceUpdateEvent)\n");

	switch (tUpdateEvent) {
		case RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS:
			printf("\tRDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS\n");
			break;
		case RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY:
			printf("\tRDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY\n");
			break;
		default:
			break;
	}
}
