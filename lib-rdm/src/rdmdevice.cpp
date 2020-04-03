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
#include <string.h>
#include <assert.h>

#include "rdmdevice.h"

#include "rdmconst.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#if defined(H3)
 #include "h3_board.h"
 static const char DEVICE_LABEL[] ALIGNED = H3_BOARD_NAME " RDM Device";
#elif defined (RASPPI) || defined (BARE_METAL)
 static const char DEVICE_LABEL[] ALIGNED = "Raspberry Pi RDM Device";
#elif defined (__CYGWIN__)
 static const char DEVICE_LABEL[] ALIGNED = "Cygwin RDM Device";
#elif defined (__linux__)
 static const char DEVICE_LABEL[] ALIGNED = "Linux RDM Device";
#elif defined (__APPLE__)
 static const char DEVICE_LABEL[] ALIGNED = "MacOS RDM Device";
#else
 static const char DEVICE_LABEL[] ALIGNED = "RDM Device";
#endif

RDMDevice::RDMDevice(void):
	m_IsInit(false),
	m_nDeviceRootLabelLength(0),
	m_nCheckSum(0),
	m_pRDMDeviceStore(0)
{
	DEBUG_ENTRY

	const uint8_t nLength = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, strlen(RDMConst::MANUFACTURER_NAME));
	memcpy(m_tRDMDevice.aDeviceManufacturerName, RDMConst::MANUFACTURER_NAME, nLength);
	m_tRDMDevice.nDdeviceManufacturerNameLength = nLength;

	m_tRDMDevice.aDeviceUID[0] = RDMConst::MANUFACTURER_ID[0];
	m_tRDMDevice.aDeviceUID[1] = RDMConst::MANUFACTURER_ID[1];

	uint8_t aMacAddress[NETWORK_MAC_SIZE];
	Network::Get()->MacAddressCopyTo(aMacAddress);

	m_tRDMDevice.aDeviceUID[2] = aMacAddress[2];
	m_tRDMDevice.aDeviceUID[3] = aMacAddress[3];
	m_tRDMDevice.aDeviceUID[4] = aMacAddress[4];
	m_tRDMDevice.aDeviceUID[5] = aMacAddress[5];

	m_tRDMDevice.aDeviceSN[0] = m_tRDMDevice.aDeviceUID[5];
	m_tRDMDevice.aDeviceSN[1] = m_tRDMDevice.aDeviceUID[4];
	m_tRDMDevice.aDeviceSN[2] = m_tRDMDevice.aDeviceUID[3];
	m_tRDMDevice.aDeviceSN[3] = m_tRDMDevice.aDeviceUID[2];

	const char* WebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	const uint8_t length = MIN(RDM_MANUFACTURER_LABEL_MAX_LENGTH, strlen(WebsiteUrl));
	memcpy(m_tRDMDevice.aDeviceManufacturerName, WebsiteUrl, length);
	m_tRDMDevice.nDdeviceManufacturerNameLength = length;

	m_tRDMDevice.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_tRDMDevice.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	m_nDeviceRootLabelLength = sizeof(DEVICE_LABEL) - 1;
	memcpy(m_aDeviceRootLabel, DEVICE_LABEL, m_nDeviceRootLabelLength);

	DEBUG_EXIT
}

RDMDevice::~RDMDevice(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

uint16_t RDMDevice::CalculateChecksum(void) {
	uint16_t nChecksum = m_nDeviceRootLabelLength;

	for (uint32_t i = 0; i < m_tRDMDevice.nDeviceRootLabelLength; i++) {
		nChecksum += (uint16_t) m_tRDMDevice.aDeviceRootLabel[i];
	}

	return nChecksum;
}

void RDMDevice::Init(void) {
	DEBUG_ENTRY

	assert(!m_IsInit);

	m_IsInit = true;

	SetFactoryDefaults();

	m_nCheckSum = CalculateChecksum();

	DEBUG_EXIT
}

void RDMDevice::SetFactoryDefaults(void) {
	DEBUG_ENTRY

	struct TRDMDeviceInfoData info;

	info.data = (uint8_t *)m_aDeviceRootLabel;
	info.length = m_nDeviceRootLabelLength;

	SetLabel(&info);

	DEBUG_EXIT
}

bool RDMDevice::GetFactoryDefaults(void) {
	return (m_nCheckSum == CalculateChecksum());
}

void RDMDevice::SetLabel(const struct TRDMDeviceInfoData *pInfo) {
	const uint8_t nLength = MIN(RDM_DEVICE_LABEL_MAX_LENGTH, pInfo->length);

	if (m_IsInit) {
		memcpy(m_tRDMDevice.aDeviceRootLabel, pInfo->data, nLength);
		m_tRDMDevice.nDeviceRootLabelLength = nLength;

		if (m_pRDMDeviceStore != 0) {
			m_pRDMDeviceStore->SaveLabel((const uint8_t *)m_tRDMDevice.aDeviceRootLabel, m_tRDMDevice.nDeviceRootLabelLength);
		}
	} else {
		memcpy(m_aDeviceRootLabel, pInfo->data, nLength);
		m_nDeviceRootLabelLength = nLength;
	}
}

void RDMDevice::GetLabel(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data = (uint8_t *) m_tRDMDevice.aDeviceRootLabel;
	pInfo->length = m_tRDMDevice.nDeviceRootLabelLength;
}

void RDMDevice::GetManufacturerId(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data[0] = RDMConst::MANUFACTURER_ID[1];
	pInfo->data[1] = RDMConst::MANUFACTURER_ID[0];
	pInfo->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void RDMDevice::GetManufacturerName(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data = (uint8_t *) m_tRDMDevice.aDeviceManufacturerName;
	pInfo->length = m_tRDMDevice.nDdeviceManufacturerNameLength;
}
