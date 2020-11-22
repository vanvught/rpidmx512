/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <cassert>

#include "rdmdevice.h"

#include "rdmconst.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

#if defined(H3)
 #include "h3_board.h"
 static constexpr char DEVICE_LABEL[] = H3_BOARD_NAME " RDM Device";
#elif defined (RASPPI) || defined (BARE_METAL)
 static constexpr char DEVICE_LABEL[] = "Raspberry Pi RDM Device";
#elif defined (__CYGWIN__)
 static constexpr char DEVICE_LABEL[] = "Cygwin RDM Device";
#elif defined (__linux__)
 static constexpr char DEVICE_LABEL[] = "Linux RDM Device";
#elif defined (__APPLE__)
 static constexpr char DEVICE_LABEL[] = "MacOS RDM Device";
#else
 static constexpr char DEVICE_LABEL[] = "RDM Device";
#endif

RDMDevice::RDMDevice() {
	DEBUG_ENTRY

	const uint8_t nLength = std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME));
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
	const uint8_t length = std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(WebsiteUrl));
	memcpy(m_tRDMDevice.aDeviceManufacturerName, WebsiteUrl, length);
	m_tRDMDevice.nDdeviceManufacturerNameLength = length;

	m_tRDMDevice.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_tRDMDevice.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	m_nDeviceRootLabelLength = sizeof(DEVICE_LABEL) - 1;
	memcpy(m_aDeviceRootLabel, DEVICE_LABEL, m_nDeviceRootLabelLength);

	DEBUG_EXIT
}

uint16_t RDMDevice::CalculateChecksum() {
	uint16_t nChecksum = m_nDeviceRootLabelLength;

	for (uint32_t i = 0; i < m_tRDMDevice.nDeviceRootLabelLength; i++) {
		nChecksum += static_cast<uint16_t>(m_tRDMDevice.aDeviceRootLabel[i]);
	}

	return nChecksum;
}

void RDMDevice::Init() {
	DEBUG_ENTRY

	assert(!m_IsInit);

	m_IsInit = true;

	SetFactoryDefaults();

	m_nCheckSum = CalculateChecksum();

	DEBUG_EXIT
}

void RDMDevice::SetFactoryDefaults() {
	DEBUG_ENTRY

	struct TRDMDeviceInfoData info;

	info.data = m_aDeviceRootLabel;
	info.length = m_nDeviceRootLabelLength;

	SetLabel(&info);

	DEBUG_EXIT
}

bool RDMDevice::GetFactoryDefaults() {
	return (m_nCheckSum == CalculateChecksum());
}

void RDMDevice::SetLabel(const struct TRDMDeviceInfoData *pInfo) {
	const uint8_t nLength = std::min(static_cast<uint8_t>(RDM_DEVICE_LABEL_MAX_LENGTH), pInfo->length);

	if (m_IsInit) {
		memcpy(m_tRDMDevice.aDeviceRootLabel, pInfo->data, nLength);
		m_tRDMDevice.nDeviceRootLabelLength = nLength;

		if (m_pRDMDeviceStore != nullptr) {
			m_pRDMDeviceStore->SaveLabel(m_tRDMDevice.aDeviceRootLabel, m_tRDMDevice.nDeviceRootLabelLength);
		}
	} else {
		memcpy(m_aDeviceRootLabel, pInfo->data, nLength);
		m_nDeviceRootLabelLength = nLength;
	}
}

void RDMDevice::GetLabel(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data = m_tRDMDevice.aDeviceRootLabel;
	pInfo->length = m_tRDMDevice.nDeviceRootLabelLength;
}

void RDMDevice::GetManufacturerId(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data[0] = static_cast<char>(RDMConst::MANUFACTURER_ID[1]);
	pInfo->data[1] = static_cast<char>(RDMConst::MANUFACTURER_ID[0]);
	pInfo->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

void RDMDevice::GetManufacturerName(struct TRDMDeviceInfoData *pInfo) {
	pInfo->data = m_tRDMDevice.aDeviceManufacturerName;
	pInfo->length = m_tRDMDevice.nDdeviceManufacturerNameLength;
}
