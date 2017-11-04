/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "rdmdevice.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "hardware.h"
#include "util.h"

static const char DEVICE_MANUFACTURER_NAME[] ALIGNED = "http://www.raspberrypi-dmx.org";
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_ID[] ALIGNED = { 0x7F, 0xF0 };	///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";
static const char RDM_DEVICE_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";
static const char RDM_DEVICE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";
static const char RDM_DEVICE_PRODUCT_CATEGORY[] ALIGNED = "product_category";
static const char RDM_DEVICE_PRODUCT_DETAIL[] ALIGNED = "product_detail";

void RDMDevice::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((RDMDevice *) p)->callbackFunction(s);
}

void RDMDevice::callbackFunction(const char *pLine) {
	assert(pLine != 0);
	char value[8];
	uint8_t len;

	len = 1;
	if (Sscan::Char(pLine, RDM_DEVICE_EXTERNAL_MONITOR, value, &len) == SSCAN_OK) {
		if (len == 1) {
			if (isdigit((int) value[0])) {
				m_nExtMonLevel = (uint8_t) (value[0] - (char) '0');
			}
		}
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_MANUFACTURER_NAME, m_aDeviceManufacturerName, &len) == SSCAN_OK) {
		m_nDdeviceManufacturerNameLength = len;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_LABEL, m_aDeviceRootLabel, &len) == SSCAN_OK) {
		m_nDeviceRootLabelLength = len;
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (Sscan::Char(pLine, RDM_DEVICE_MANUFACTURER_ID, value, &len) == SSCAN_OK) {
		if (len == 4) {
			const uint16_t v = (uint16_t) hex_uint32(value);
			m_aDeviceUID[0] = (uint8_t) (v >> 8);
			m_aDeviceUID[1] = (uint8_t) (v & 0xFF);
		}
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (Sscan::Char(pLine, RDM_DEVICE_PRODUCT_CATEGORY, value, &len) == SSCAN_OK) {
		if (len == 4) {
			m_nProductCategory = (uint16_t) hex_uint32(value);
		}
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (Sscan::Char(pLine, RDM_DEVICE_PRODUCT_DETAIL, value, &len) == SSCAN_OK) {
		if (len == 4) {
			m_nProductDetail = (uint16_t) hex_uint32(value);
		}
		return;
	}
}

RDMDevice::RDMDevice(void): m_bSetList(0) {
	uint8_t mac_address[6];

	if (hardware_get_mac_address(mac_address) == 0) {
		m_aDeviceUID[2] = mac_address[2];
		m_aDeviceUID[3] = mac_address[3];
		m_aDeviceUID[4] = mac_address[4];
		m_aDeviceUID[5] = mac_address[5];
	}

	m_aDeviceUID[0] = DEVICE_MANUFACTURER_ID[0];
	m_aDeviceUID[1] = DEVICE_MANUFACTURER_ID[1];

	m_aDeviceSN[0] = m_aDeviceUID[5];
	m_aDeviceSN[1] = m_aDeviceUID[4];
	m_aDeviceSN[2] = m_aDeviceUID[3];
	m_aDeviceSN[3] = m_aDeviceUID[2];

	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, DEVICE_MANUFACTURER_NAME_LENGTH);
	memcpy(m_aDeviceManufacturerName, DEVICE_MANUFACTURER_NAME, length);
	m_nDdeviceManufacturerNameLength = length;

	memset(m_aDeviceRootLabel, 0, RDM_DEVICE_LABEL_MAX_LENGTH);
	m_nDeviceRootLabelLength = 0;

	m_nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	m_nExtMonLevel = 0;
}

RDMDevice::~RDMDevice(void) {
}

void RDMDevice::GetLabel(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)m_aDeviceRootLabel;
	info->length = m_nDeviceRootLabelLength;
}

void RDMDevice::SetLabel(const struct _rdm_device_info_data *info) {
	const uint8_t length = MIN(RDM_DEVICE_LABEL_MAX_LENGTH, info->length);
	(void *)memcpy(m_aDeviceRootLabel, info->data, length);
	m_nDeviceRootLabelLength = length;
}

void RDMDevice::GetManufacturerId(struct _rdm_device_info_data *info) {
	info->data[0] = m_aDeviceUID[1];
	info->data[1] = m_aDeviceUID[0];
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void RDMDevice::SetManufacturerId(const struct _rdm_device_info_data *info) {
	if (info->length != RDM_DEVICE_MANUFACTURER_ID_LENGTH) {
		return;
	}

	m_aDeviceUID[1] = info->data[0];
	m_aDeviceUID[0] = info->data[1];
}

const uint8_t* RDMDevice::GetUID(void) const {
	return (const uint8_t *)m_aDeviceUID;
}

void RDMDevice::GetManufacturerName(struct _rdm_device_info_data *info){
	info->data = (uint8_t *)m_aDeviceManufacturerName;
	info->length = m_nDdeviceManufacturerNameLength;
}

void RDMDevice::SetManufacturerName(const struct _rdm_device_info_data *info) {
	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, info->length);
	(void *)memcpy(m_aDeviceManufacturerName, info->data, length);
	m_nDdeviceManufacturerNameLength = length;
}

bool RDMDevice::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(RDMDevice::staticCallbackFunction, this);
	return configfile.Read(RDM_DEVICE_FILE_NAME);
}

void RDMDevice::Dump(void) {
	if (m_bSetList == 0) {
		return;
	}

	printf("RDM Device parameters \'%s\':\n", RDM_DEVICE_FILE_NAME);
}

uint8_t RDMDevice::GetExtMonLevel(void) const {
	return m_nExtMonLevel;
}

bool RDMDevice::isMaskSet(uint16_t mask) {
	return (m_bSetList & mask) == mask;
}

#if defined (__circle__)
void RDMDevice::printf(const char *fmt, ...) {
	assert(fmt != 0);

	size_t fmtlen = strlen(fmt);
	char fmtbuf[fmtlen + 1];

	strcpy(fmtbuf, fmt);

	if (fmtbuf[fmtlen - 1] == '\n') {
		fmtbuf[fmtlen - 1] = '\0';
	}

	va_list var;
	va_start(var, fmt);

	CLogger::Get()->WriteV("", LogNotice, fmtbuf, var);

	va_end(var);
}
#endif
