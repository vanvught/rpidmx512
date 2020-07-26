/**
 * @file rdmdeviceresponderprint.cpp
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

#include <stdio.h>

#include "rdmdeviceresponder.h"
#include "rdmdevice.h"

void RDMDeviceResponder::Print() {
	RDMDevice::Print();

	const struct TRDMDeviceInfo *pDeviceInfo = GetDeviceInfo();
	const char *pPersonalityDescription = m_pRDMPersonality->GetDescription();
	const uint8_t nPersonalityDescriptionLength = m_pRDMPersonality->GetDescriptionLength();

	printf("RDM Responder configuration\n");
	printf(" Protocol Version %d.%d\n", pDeviceInfo->protocol_major, pDeviceInfo->protocol_minor);
	printf(" DMX Address      : %d\n", (pDeviceInfo->dmx_start_address[0] << 8) + pDeviceInfo->dmx_start_address[1]);
	printf(" DMX Footprint    : %d\n", (pDeviceInfo->dmx_footprint[0] << 8) + pDeviceInfo->dmx_footprint[1]);
	printf(" Personality %d of %d [%.*s]\n", pDeviceInfo->current_personality, pDeviceInfo->personality_count, nPersonalityDescriptionLength, pPersonalityDescription);
	printf(" Sub Devices      : %d\n", (pDeviceInfo->sub_device_count[0] << 8) + pDeviceInfo->sub_device_count[1]);
	printf(" Sensors          : %d\n", pDeviceInfo->sensor_count);
}
