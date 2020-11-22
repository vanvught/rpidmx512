/**
 * @file rdm_device_info.cpp
 *
 * @brief Bridge between C++ and C
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <assert.h>

#include "rdm.h"
#include "rdm_device_info.h"

#include "rdmdevice.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static uint8_t uid_device[RDM_UID_SIZE] = { 0x50, 0x00, 0x00, 0x00, 0x00, 0x00 };

static char root_device_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;
static uint8_t root_device_label_length =  0;

static char device_manufacturer_name[RDM_MANUFACTURER_LABEL_MAX_LENGTH] ALIGNED;
static uint8_t device_manufacturer_name_length = 0;

static uint8_t manufacturer_id[RDM_DEVICE_MANUFACTURER_ID_LENGTH] ALIGNED;

static uint8_t device_sn[DEVICE_SN_LENGTH] ALIGNED;

void rdm_device_info_init(RDMDevice *pRdmDevice) {
	assert(pRdmDevice != nullptr);

	struct TRDMDeviceInfoData info;

	const uint8_t *pUID = pRdmDevice->GetUID();
	memcpy(uid_device, pUID, RDM_UID_SIZE);

	pRdmDevice->GetLabel(&info);
	memcpy(root_device_label, info.data, info.length);
	root_device_label_length = info.length;

	pRdmDevice->GetManufacturerName(&info);
	memcpy(device_manufacturer_name, info.data, info.length);
	device_manufacturer_name_length = info.length;

	const uint8_t *pSN = pRdmDevice->GetSN();
	memcpy(device_sn, pSN, DEVICE_SN_LENGTH);
}

void rdm_device_info_get_label(__attribute__((unused))  uint16_t sub_device, struct _rdm_device_info_data *info) {
	info->data = reinterpret_cast<uint8_t*>(root_device_label);
	info->length = root_device_label_length;
}

void rdm_device_info_get_manufacturer_name(struct _rdm_device_info_data *info) {
	info->data = reinterpret_cast<uint8_t*>(device_manufacturer_name);
	info->length = device_manufacturer_name_length;
}

void rdm_device_info_get_manufacturer_id(struct _rdm_device_info_data *info) {
	manufacturer_id[0] = uid_device[1];
	manufacturer_id[1] = uid_device[0];

	info->data = const_cast<uint8_t*>(manufacturer_id);
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void rdm_device_info_get_sn(struct _rdm_device_info_data *info) {
	info->data = const_cast<uint8_t*>(device_sn);
	info->length = DEVICE_SN_LENGTH;
}
