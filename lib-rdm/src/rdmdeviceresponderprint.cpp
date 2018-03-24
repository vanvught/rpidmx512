/**
 * @file rdmdeviceresponderprint.cpp
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

#include "rdmdeviceresponder.h"
#include "rdm.h"

void RDMDeviceResponder::Print(void) {
	const uint8_t *pUuid = GetUID();
	const struct TRDMDeviceInfo *info = GetDeviceInfo();
	const char* pPersonalityDescription = m_pRDMPersonality->GetDescription();
	const uint8_t nPersonalityDescriptionLength = m_pRDMPersonality->GetDescriptionLength();

	struct TRDMDeviceInfoData InfoData;
	GetLabel(RDM_ROOT_DEVICE, &InfoData);

	printf("\nRDM Responder configuration\n");
	printf(" Protocol Version %d.%d\n", (int) info->protocol_major, (int) info->protocol_minor);
	printf(" DMX Address   : %d\n", (int) (((uint16_t) info->dmx_start_address[0] << 8) + info->dmx_start_address[1]));
	printf(" DMX Footprint : %d\n", (int) (((uint16_t) info->dmx_footprint[0] << 8) + info->dmx_footprint[1]));
	printf(" Personality %d of %d [%.*s]\n", (int) info->current_personality, (int) info->personality_count, nPersonalityDescriptionLength, pPersonalityDescription);
	printf(" Sub Devices   : %d\n", (int) (((uint16_t) info->sub_device_count[0] << 8) + info->sub_device_count[1]));
	printf(" Sensors       : %d\n", (int) info->sensor_count);
	printf(" UUID          : %.2x%.2x:%.2x%.2x%.2x%.2x\n", pUuid[0], pUuid[1], pUuid[2], pUuid[3], pUuid[4], pUuid[5]);
	printf(" Device Label  : %.*s\n", InfoData.length, InfoData.data);
}
