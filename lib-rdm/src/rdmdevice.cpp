/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <ctype.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "rdmdevice.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "network.h"
#include "hardware.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

static const uint8_t DEVICE_MANUFACTURER_ID[] ALIGNED = { 0x7F, 0xF0 };	///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";
static const char RDM_DEVICE_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";
static const char RDM_DEVICE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";
static const char RDM_DEVICE_PRODUCT_CATEGORY[] ALIGNED = "product_category";
static const char RDM_DEVICE_PRODUCT_DETAIL[] ALIGNED = "product_detail";

RDMDevice::RDMDevice(const struct TRDMDeviceInfoData *tLabel, RDMDeviceStore* m_pRDMDeviceParamsStore): m_pRDMDeviceParamsStore(m_pRDMDeviceParamsStore) {
	m_tRDMDeviceParams.nSetList = 0;

	uint8_t aMacAddress[NETWORK_MAC_SIZE];

	assert((uint32_t) Network::Get());
	Network::Get()->MacAddressCopyTo(aMacAddress);

	m_tRDMDeviceParams.aDeviceUID[0] = DEVICE_MANUFACTURER_ID[0];
	m_tRDMDeviceParams.aDeviceUID[1] = DEVICE_MANUFACTURER_ID[1];

	m_tRDMDeviceParams.aDeviceUID[2] = aMacAddress[2];
	m_tRDMDeviceParams.aDeviceUID[3] = aMacAddress[3];
	m_tRDMDeviceParams.aDeviceUID[4] = aMacAddress[4];
	m_tRDMDeviceParams.aDeviceUID[5] = aMacAddress[5];

	m_tRDMDeviceParams.aDeviceSN[0] = m_tRDMDeviceParams.aDeviceUID[5];
	m_tRDMDeviceParams.aDeviceSN[1] = m_tRDMDeviceParams.aDeviceUID[4];
	m_tRDMDeviceParams.aDeviceSN[2] = m_tRDMDeviceParams.aDeviceUID[3];
	m_tRDMDeviceParams.aDeviceSN[3] = m_tRDMDeviceParams.aDeviceUID[2];

	assert((uint32_t) Hardware::Get());
	const char* WebsiteUrl = Hardware::Get()->GetWebsiteUrl();

	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, strlen(WebsiteUrl));
	memcpy(m_tRDMDeviceParams.aDeviceManufacturerName, WebsiteUrl, length);
	m_tRDMDeviceParams.nDdeviceManufacturerNameLength = length;

	memset(m_tRDMDeviceParams.aDeviceRootLabel, 0, RDM_DEVICE_LABEL_MAX_LENGTH);
	m_tRDMDeviceParams.nDeviceRootLabelLength = 0;

	if (tLabel != 0) {
		memcpy(m_tRDMDeviceParams.aDeviceRootLabel, tLabel->data, tLabel->length);
		m_tRDMDeviceParams.nDeviceRootLabelLength = tLabel->length;
	}

	m_tRDMDeviceParams.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_tRDMDeviceParams.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	m_tRDMDeviceParams.nExtMonLevel = 0;
}

RDMDevice::~RDMDevice(void) {
	m_tRDMDeviceParams.nSetList = 0;
}

bool RDMDevice::Load(void) {
	m_tRDMDeviceParams.nSetList = 0;

	ReadConfigFile configfile(RDMDevice::staticCallbackFunction, this);

	if (configfile.Read(RDM_DEVICE_FILE_NAME)) {
		// There is a configuration file
		if (m_pRDMDeviceParamsStore != 0) {
			m_pRDMDeviceParamsStore->Update(&m_tRDMDeviceParams);
		}
	} else if (m_pRDMDeviceParamsStore != 0) {
		m_pRDMDeviceParamsStore->Copy(&m_tRDMDeviceParams);
	} else {
		return false;
	}

	return true;
}

void RDMDevice::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pRDMDeviceParamsStore != 0);

	if (m_pRDMDeviceParamsStore == 0) {
		return;
	}

	m_tRDMDeviceParams.nSetList = 0;

	ReadConfigFile config(RDMDevice::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRDMDeviceParamsStore->Update(&m_tRDMDeviceParams);
}

void RDMDevice::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t len;
	uint8_t uint8;
	uint16_t uint16;

	if (Sscan::Uint8(pLine, RDM_DEVICE_EXTERNAL_MONITOR, &uint8) == SSCAN_OK) {
		m_tRDMDeviceParams.nExtMonLevel = uint8;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_EXTERNAL_MONITOR;
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_MANUFACTURER_NAME, m_tRDMDeviceParams.aDeviceManufacturerName, &len) == SSCAN_OK) {
		m_tRDMDeviceParams.nDdeviceManufacturerNameLength = len;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_MANUFACTURER_NAME;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (Sscan::Char(pLine, RDM_DEVICE_LABEL, m_tRDMDeviceParams.aDeviceRootLabel, &len) == SSCAN_OK) {
		m_tRDMDeviceParams.nDeviceRootLabelLength = len;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_LABEL;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_MANUFACTURER_ID, &uint16) == SSCAN_OK) {
		m_tRDMDeviceParams.aDeviceUID[0] = (uint8_t) (uint16 >> 8);
		m_tRDMDeviceParams.aDeviceUID[1] = (uint8_t) (uint16 & 0xFF);
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_MANUFACTURER_ID;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_PRODUCT_CATEGORY, &uint16) == SSCAN_OK) {
		m_tRDMDeviceParams.nProductCategory = uint16;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_PRODUCT_CATEGORY;
		return;
	}

	if (Sscan::HexUint16(pLine, RDM_DEVICE_PRODUCT_DETAIL, &uint16) == SSCAN_OK) {
		m_tRDMDeviceParams.nProductDetail = uint16;
		m_tRDMDeviceParams.nSetList |= RDMDEVICE_MASK_PRODUCT_DETAIL;
	}
}

void RDMDevice::Dump(void) {
#ifndef NDEBUG
	if (m_tRDMDeviceParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, RDM_DEVICE_FILE_NAME);


	if (isMaskSet(RDMDEVICE_MASK_LABEL)) {
		printf(" %s=%.*s\n", RDM_DEVICE_LABEL, m_tRDMDeviceParams.nDeviceRootLabelLength, m_tRDMDeviceParams.aDeviceRootLabel);
	}

	if (isMaskSet(RDMDEVICE_MASK_MANUFACTURER_NAME)) {
		printf(" %s=%.*s\n", RDM_DEVICE_MANUFACTURER_NAME, m_tRDMDeviceParams.nDdeviceManufacturerNameLength, m_tRDMDeviceParams.aDeviceManufacturerName);
	}

	if (isMaskSet(RDMDEVICE_MASK_MANUFACTURER_ID)) {
		printf(" %s=%x%x\n", RDM_DEVICE_MANUFACTURER_ID, m_tRDMDeviceParams.aDeviceUID[0], m_tRDMDeviceParams.aDeviceUID[1]);
	}

	if (isMaskSet(RDMDEVICE_MASK_PRODUCT_CATEGORY)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_CATEGORY, m_tRDMDeviceParams.nProductCategory);
	}

	if (isMaskSet(RDMDEVICE_MASK_PRODUCT_DETAIL)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_DETAIL, m_tRDMDeviceParams.nProductDetail);
	}

	if (isMaskSet(RDMDEVICE_MASK_PRODUCT_CATEGORY)) {
		printf(" %s=%x\n", RDM_DEVICE_PRODUCT_CATEGORY, m_tRDMDeviceParams.nProductCategory);
	}
#endif
}

void RDMDevice::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((RDMDevice *) p)->callbackFunction(s);
}

bool RDMDevice::isMaskSet(uint32_t nMask) {
	return (m_tRDMDeviceParams.nSetList & nMask) == nMask;
}

