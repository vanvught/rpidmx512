/**
 * @file rdmdevice.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

RDMDevice *RDMDevice::s_pThis;

RDMDevice::RDMDevice() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aUID[0] = RDMConst::MANUFACTURER_ID[0];
	m_aUID[1] = RDMConst::MANUFACTURER_ID[1];

#if defined (NO_EMAC) || !defined(CONFIG_RDMDEVICE_UUID_IP)
	uint8_t aMacAddress[network::MAC_SIZE];
	Network::Get()->MacAddressCopyTo(aMacAddress);

	m_aUID[2] = aMacAddress[2];
	m_aUID[3] = aMacAddress[3];
	m_aUID[4] = aMacAddress[4];
	m_aUID[5] = aMacAddress[5];

	m_aSN[0] = m_aUID[5];
	m_aSN[1] = m_aUID[4];
	m_aSN[2] = m_aUID[3];
	m_aSN[3] = m_aUID[2];
#else
	// Calculated with GetUID and GetSN
#endif

	m_nFactoryRootLabelLength = sizeof(DEVICE_LABEL) - 1;
	memcpy(m_aFactoryRootLabel, DEVICE_LABEL, m_nFactoryRootLabelLength);

	DEBUG_EXIT
}

void RDMDevice::Print() {
	puts("RDM Device configuration");
	const auto nLength = static_cast<uint8_t>(std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME)));
	printf(" Manufacturer Name : %.*s\n", nLength, const_cast<char *>(&RDMConst::MANUFACTURER_NAME[0]));
	printf(" Manufacturer ID   : %.2X%.2X\n", m_aUID[0], m_aUID[1]);
	printf(" Serial Number     : %.2X%.2X%.2X%.2X\n", m_aSN[3], m_aSN[2], m_aSN[1], m_aSN[0]);
	printf(" Root label        : %.*s\n", m_nRootLabelLength, m_aRootLabel);
	printf(" Product Category  : %.2X%.2X\n", m_nProductCategory >> 8, m_nProductCategory & 0xFF);
	printf(" Product Detail    : %.2X%.2X\n", m_nProductDetail >> 8, m_nProductDetail & 0xFF);
}
