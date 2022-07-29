/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "rdmdevice.h"
#include "rdmconst.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

#if defined (CONFIG_RDM_DEVICE_ROOT_LABEL)
 static constexpr char DEVICE_LABEL[] = CONFIG_RDM_DEVICE_ROOT_LABEL;
#else
# if defined (H3)
#  include "h3_board.h"
   static constexpr char DEVICE_LABEL[] = H3_BOARD_NAME " RDM Device";
# elif defined (GD32)
#  include "gd32_board.h"
   static constexpr char DEVICE_LABEL[] = GD32_BOARD_NAME " RDM Device";
# elif defined (RASPPI)
   static constexpr char DEVICE_LABEL[] = "Raspberry Pi RDM Device";
# elif defined (__CYGWIN__)
   static constexpr char DEVICE_LABEL[] = "Cygwin RDM Device";
# elif defined (__linux__)
   static constexpr char DEVICE_LABEL[] = "Linux RDM Device";
# elif defined (__APPLE__)
   static constexpr char DEVICE_LABEL[] = "MacOS RDM Device";
# else
   static constexpr char DEVICE_LABEL[] = "RDM Device";
# endif
#endif

RDMDevice::RDMDevice() {
	DEBUG_ENTRY

	const auto nLength = std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME));
	memcpy(m_tRDMDevice.aDeviceManufacturerName, RDMConst::MANUFACTURER_NAME, nLength);
	m_tRDMDevice.nDdeviceManufacturerNameLength = static_cast<uint8_t>(nLength);

	m_tRDMDevice.aDeviceUID[0] = RDMConst::MANUFACTURER_ID[0];
	m_tRDMDevice.aDeviceUID[1] = RDMConst::MANUFACTURER_ID[1];

	uint8_t aMacAddress[network::MAC_SIZE];
	Network::Get()->MacAddressCopyTo(aMacAddress);

	m_tRDMDevice.aDeviceUID[2] = aMacAddress[2];
	m_tRDMDevice.aDeviceUID[3] = aMacAddress[3];
	m_tRDMDevice.aDeviceUID[4] = aMacAddress[4];
	m_tRDMDevice.aDeviceUID[5] = aMacAddress[5];

	m_tRDMDevice.aDeviceSN[0] = m_tRDMDevice.aDeviceUID[5];
	m_tRDMDevice.aDeviceSN[1] = m_tRDMDevice.aDeviceUID[4];
	m_tRDMDevice.aDeviceSN[2] = m_tRDMDevice.aDeviceUID[3];
	m_tRDMDevice.aDeviceSN[3] = m_tRDMDevice.aDeviceUID[2];

	const auto* WebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	const auto length = std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(WebsiteUrl));
	memcpy(m_tRDMDevice.aDeviceManufacturerName, WebsiteUrl, length);
	m_tRDMDevice.nDdeviceManufacturerNameLength = static_cast<uint8_t>(length);

	m_tRDMDevice.nProductCategory = E120_PRODUCT_CATEGORY_OTHER;
	m_tRDMDevice.nProductDetail = E120_PRODUCT_DETAIL_OTHER;

	m_nDeviceRootLabelLength = sizeof(DEVICE_LABEL) - 1;
	memcpy(m_aDeviceRootLabel, DEVICE_LABEL, m_nDeviceRootLabelLength);

	DEBUG_EXIT
}

void RDMDevice::Print() {
	printf("RDM Device configuration\n");
	printf(" Manufacturer Name : %.*s\n", m_tRDMDevice.nDdeviceManufacturerNameLength, m_tRDMDevice.aDeviceManufacturerName);
	printf(" Manufacturer ID   : %.2X%.2X\n", m_tRDMDevice.aDeviceUID[0], m_tRDMDevice.aDeviceUID[1]);
	printf(" Serial Number     : %.2X%.2X%.2X%.2X\n", m_tRDMDevice.aDeviceSN[3], m_tRDMDevice.aDeviceSN[2], m_tRDMDevice.aDeviceSN[1], m_tRDMDevice.aDeviceSN[0]);
	printf(" Root label        : %.*s\n", m_tRDMDevice.nDeviceRootLabelLength, m_tRDMDevice.aDeviceRootLabel);
	printf(" Product Category  : %.2X%.2X\n", m_tRDMDevice.nProductCategory >> 8, m_tRDMDevice.nProductCategory & 0xFF);
	printf(" Product Detail    : %.2X%.2X\n", m_tRDMDevice.nProductDetail >> 8, m_tRDMDevice.nProductDetail & 0xFF);
}
