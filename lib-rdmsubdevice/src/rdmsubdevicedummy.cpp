/**
 * @file rdmsubdevicedummy.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "rdmsubdevicedummy.h"
#include "rdmpersonality.h"

#define DMX_FOOTPRINT	4

static RDMPersonality *s_RDMPersonalities[] = {new RDMPersonality("Dummy-H", DMX_FOOTPRINT), new RDMPersonality("Dummy-D", DMX_FOOTPRINT)};

RDMSubDeviceDummy::RDMSubDeviceDummy(uint16_t nDmxStartAddress, char nChipSselect, uint8_t nSlaveAddress, uint32_t nSpiSpeed) :
	RDMSubDevice("SubDevice Dummy", nDmxStartAddress)
{
	SetDmxFootprint(DMX_FOOTPRINT);
	SetPersonalities(s_RDMPersonalities, 2);
}

RDMSubDeviceDummy::~RDMSubDeviceDummy(void) {
}

bool RDMSubDeviceDummy::Initialize(void) {
	return true;
}

void RDMSubDeviceDummy::Start(void) {
	printf("RDMSubDeviceDummy::Start(void)\n");
}

void RDMSubDeviceDummy::Stop(void) {
	printf("RDMSubDeviceDummy::Stop(void)\n");
}

void RDMSubDeviceDummy::Data(const uint8_t* pData, uint16_t nLength) {
	assert(pData != 0);
	assert(nLength <= 512);

	const uint16_t nDmxStartAddress = GetDmxStartAddress();

	printf("RDMSubDeviceDummy::Data(*pData:%p, nLength:%d)\n", (void *) pData, (int) nLength);
	printf("%d:%d:%d: ", (int) nLength, (int) DMX_FOOTPRINT, (int) nDmxStartAddress);

	for (uint16_t i = nDmxStartAddress - 1, j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {

		switch (GetPersonalityCurrent()) {
			case 1:
				printf("%.2x ", pData[i]);
				break;
			case 2:
				printf("%.3d ", (int) pData[i]);
				break;
			default:
				break;
		}
	}

	printf("\n");
}

void RDMSubDeviceDummy::UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) {
	printf("RDMSubDeviceDummy::UpdateEvent(TRDMSubDeviceUpdateEvent)\n");

	switch (tUpdateEvent) {
		case RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS:
			printf("\tRDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS");
			break;
		case RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY:
			printf("\tRDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY");
			break;
		default:
			break;
	}
}
