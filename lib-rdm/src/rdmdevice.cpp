/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <ctype.h>
#include <assert.h>

#include "rdmdevice.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "readconfigfile.h"
#include "sscan.h"

 #include "network.h"

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define MANUFACTURER_NAME_MASK	1<<0
#define MANUFACTURER_ID_MASK	1<<1
#define LABEL_MASK				1<<2
#define EXTERNAL_MONITOR_MASK	1<<3
#define PRODUCT_CATEGORY_MASK	1<<4
#define PRODUCT_DETAIL_MASK		1<<5

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

	char value[4];
	uint8_t len;
	uint16_t uint16;

	len = 1;
	if (Sscan::Char(pLine, RDM_DEVICE_EXTERNAL_MONITOR, value, &len) == SSCAN_OK) {
		if (len == 1) {
			if (isdigit((int) value[0])) {
				m_nExtMonLevel = (uint8_t) (value[0] - (char) '0');
				m_bSetList |= EXTERNAL_MONITOR_MASK;
			}
		}
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_MANUFACTURER_NAME, m_aDeviceManufacturerName, &len) == SSCAN_OK) {
		m_nDdeviceManufacturerNameLength = len;
		m_bSetList |= MANUFACTURER_NAME_MASK;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_LABEL, m_aDeviceRootLabel, &len) == SSCAN_OK) {
		m_nDeviceRootLabelLength = len;
		m_bSetList |= LABEL_MASK;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_MANUFACTURER_ID, &uint16) == SSCAN_OK) {
		m_aDeviceUID[0] = (uint8_t) (uint16 >> 8);
		m_aDeviceUID[1] = (uint8_t) (uint16 & 0xFF);
		m_bSetList |= MANUFACTURER_ID_MASK;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_PRODUCT_CATEGORY, &uint16) == SSCAN_OK) {
		m_nProductCategory = uint16;
		m_bSetList |= PRODUCT_CATEGORY_MASK;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_PRODUCT_DETAIL, &uint16) == SSCAN_OK) {
		m_nProductDetail = uint16;
		m_bSetList |= PRODUCT_DETAIL_MASK;
	}
}

RDMDevice::RDMDevice(void): m_bSetList(0) {
	uint8_t aMacAddress[NETWORK_MAC_SIZE];

	assert(Network::Get());
	Network::Get()->MacAddressCopyTo(aMacAddress);

	m_aDeviceUID[0] = DEVICE_MANUFACTURER_ID[0];
	m_aDeviceUID[1] = DEVICE_MANUFACTURER_ID[1];

	m_aDeviceUID[2] = aMacAddress[2];
	m_aDeviceUID[3] = aMacAddress[3];
	m_aDeviceUID[4] = aMacAddress[4];
	m_aDeviceUID[5] = aMacAddress[5];

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

void RDMDevice::GetLabel(struct TRDMDeviceInfoData *info) {
	info->data = (uint8_t *)m_aDeviceRootLabel;
	info->length = m_nDeviceRootLabelLength;
}

void RDMDevice::SetLabel(const struct TRDMDeviceInfoData *info) {
	const uint8_t length = MIN(RDM_DEVICE_LABEL_MAX_LENGTH, info->length);
	memcpy(m_aDeviceRootLabel, info->data, length);
	m_nDeviceRootLabelLength = length;
}

void RDMDevice::GetManufacturerId(struct TRDMDeviceInfoData *info) {
	info->data[0] = m_aDeviceUID[1];
	info->data[1] = m_aDeviceUID[0];
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void RDMDevice::SetManufacturerId(const struct TRDMDeviceInfoData *info) {
	if (info->length != RDM_DEVICE_MANUFACTURER_ID_LENGTH) {
		return;
	}

	m_aDeviceUID[1] = info->data[0];
	m_aDeviceUID[0] = info->data[1];
}

const uint8_t* RDMDevice::GetUID(void) const {
	return (const uint8_t *)m_aDeviceUID;
}

const uint8_t* RDMDevice::GetSN(void) const {
	return (const uint8_t *)m_aDeviceSN;
}

void RDMDevice::GetManufacturerName(struct TRDMDeviceInfoData *info){
	info->data = (uint8_t *)m_aDeviceManufacturerName;
	info->length = m_nDdeviceManufacturerNameLength;
}

void RDMDevice::SetManufacturerName(const struct TRDMDeviceInfoData *info) {
	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, info->length);
	memcpy(m_aDeviceManufacturerName, info->data, length);
	m_nDdeviceManufacturerNameLength = length;
}

uint16_t RDMDevice::GetProductCategory(void) const {
	return m_nProductCategory;
}

uint16_t RDMDevice::GetProductDetail(void) const {
	return m_nProductDetail;
}

bool RDMDevice::Load(void) {
	m_bSetList = 0;

	ReadConfigFile configfile(RDMDevice::staticCallbackFunction, this);
	return configfile.Read(RDM_DEVICE_FILE_NAME);
}

void RDMDevice::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, RDM_DEVICE_FILE_NAME);


	if (isMaskSet(LABEL_MASK)) {
		printf(" %s=%.*s\n", RDM_DEVICE_LABEL, m_nDeviceRootLabelLength, m_aDeviceRootLabel);
	}

	if (isMaskSet(MANUFACTURER_NAME_MASK)) {
		printf(" %s=%.*s\n", RDM_DEVICE_MANUFACTURER_NAME, m_nDdeviceManufacturerNameLength, m_aDeviceManufacturerName);
	}

	if (isMaskSet(MANUFACTURER_ID_MASK)) {
		printf(" %s=%x%x\n", RDM_DEVICE_MANUFACTURER_ID, m_aDeviceUID[0], m_aDeviceUID[1]);
	}

	if (isMaskSet(PRODUCT_CATEGORY_MASK)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_CATEGORY, m_nProductCategory);
	}

	if (isMaskSet(PRODUCT_DETAIL_MASK)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_DETAIL, m_nProductDetail);
	}

	if (isMaskSet(PRODUCT_CATEGORY_MASK)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_CATEGORY, m_nProductCategory);
	}

#endif
}

uint8_t RDMDevice::GetExtMonLevel(void) const {
	return m_nExtMonLevel;
}

bool RDMDevice::isMaskSet(uint32_t nMask) {
	return (m_bSetList & nMask) == nMask;
}

