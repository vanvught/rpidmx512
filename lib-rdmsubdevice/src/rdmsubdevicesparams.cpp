/**
 * @file rdmsubdevicesparams.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cassert>

#include "rdmsubdevicesparams.h"
#include "rdmsubdevices.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#if defined(RDM_SUBDEVICES_ENABLE)
# include "rdmsubdevicebw7fets.h"
# include "rdmsubdevicebwdimmer.h"
# include "rdmsubdevicebwdio.h"
# include "rdmsubdevicebwlcd.h"
# include "rdmsubdevicebwrelay.h"
//
# include "rdmsubdevicemcp23s08.h"
# include "rdmsubdevicemcp23s17.h"
//
# include "rdmsubdevicemcp4822.h"
# include "rdmsubdevicemcp4902.h"
#endif

#include "debug.h"

using namespace rdm::subdevices;

RDMSubDevicesParams::RDMSubDevicesParams(RDMSubDevicesParamsStore *pRDMSubDevicesParamsStore): m_pRDMSubDevicesParamsStore(pRDMSubDevicesParamsStore) {
	DEBUG_PRINTF("sizeof(struct TRDMSubDevicesParams)=%d", static_cast<int>(sizeof(struct TRDMSubDevicesParams)));
}

bool RDMSubDevicesParams::Load() {
	DEBUG_ENTRY

	m_tRDMSubDevicesParams.nCount = 0;

	ReadConfigFile configfile(RDMSubDevicesParams::staticCallbackFunction, this);

	if (configfile.Read(RDMSubDevicesConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pRDMSubDevicesParamsStore != nullptr) {
			m_pRDMSubDevicesParamsStore->Update(&m_tRDMSubDevicesParams);
		}
	} else if (m_pRDMSubDevicesParamsStore != nullptr) {
		m_pRDMSubDevicesParamsStore->Copy(&m_tRDMSubDevicesParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void RDMSubDevicesParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pRDMSubDevicesParamsStore != nullptr);

	if (m_pRDMSubDevicesParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_tRDMSubDevicesParams.nCount = 0;

	ReadConfigFile config(RDMSubDevicesParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRDMSubDevicesParamsStore->Update(&m_tRDMSubDevicesParams);

	DEBUG_EXIT
}

void RDMSubDevicesParams::Builder(const struct TRDMSubDevicesParams *pRDMSubDevicesParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pRDMSubDevicesParams != nullptr) {
		memcpy(&m_tRDMSubDevicesParams, pRDMSubDevicesParams, sizeof(struct TRDMSubDevicesParams));
	} else {
		assert(m_pRDMSubDevicesParamsStore != nullptr);
		m_pRDMSubDevicesParamsStore->Copy(&m_tRDMSubDevicesParams);
	}

	PropertiesBuilder builder(RDMSubDevicesConst::PARAMS_FILE_NAME, pBuffer, nLength);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void RDMSubDevicesParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pRDMSubDevicesParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}

void RDMSubDevicesParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RDMSubDevicesConst::PARAMS_FILE_NAME);

	for (uint32_t i = 0; i < m_tRDMSubDevicesParams.nCount; i++) {
		printf(" %s 0x%.2x\n", RDMSubDevices::GetTypeString(static_cast<type>(m_tRDMSubDevicesParams.Entry[i].nType)), m_tRDMSubDevicesParams.Entry[i].nAddress);
	}
#endif
}

bool RDMSubDevicesParams::Add(RDMSubDevice *pRDMSubDevice) {
	DEBUG_ENTRY

	if (pRDMSubDevice->Initialize()) {
		RDMSubDevices::Get()->Add(pRDMSubDevice);
		DEBUG_EXIT
		return true;
	}

	delete pRDMSubDevice;

	DEBUG_EXIT
	return false;
}

void RDMSubDevicesParams::Set() {
#if defined(RDM_SUBDEVICES_ENABLE)
	for (uint32_t i = 0; i < m_tRDMSubDevicesParams.nCount; i++) {
		const auto nChipSelect = m_tRDMSubDevicesParams.Entry[i].nChipSelect;
		const auto nAddress = m_tRDMSubDevicesParams.Entry[i].nAddress;
		const auto nDmxStartAddress = m_tRDMSubDevicesParams.Entry[i].nDmxStartAddress;
		const auto nSpeedHz = m_tRDMSubDevicesParams.Entry[i].nSpeedHz;

		switch (static_cast<type>(m_tRDMSubDevicesParams.Entry[i].nType)) {
			case type::BW7FETS:
				Add(new RDMSubDeviceBw7fets(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::BWDIMMER:
				Add(new RDMSubDeviceBwDimmer(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::BWDIO:
				Add(new RDMSubDeviceBwDio(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::BWLCD:
				Add(new RDMSubDeviceBwLcd(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::BWRELAY:
				Add(new RDMSubDeviceBwRelay(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::MCP23S08:
				Add(new RDMSubDeviceMCP23S08(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::MCP23S17:
				Add(new RDMSubDeviceMCP23S17(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::MCP4822:
				Add(new RDMSubDeviceMCP4822(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			case type::MCP4902:
				Add(new RDMSubDeviceMCP4902(nDmxStartAddress, nChipSelect, nAddress, nSpeedHz));
				break;
			default:
				break;
		}
	}
#endif
}

void RDMSubDevicesParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aSubDeviceName[32];
	memset(aSubDeviceName, 0, sizeof(aSubDeviceName));

	char nChipSelect = 0;
	uint8_t nLength = sizeof(aSubDeviceName) - 1;
	uint8_t nAddress = 0;
	uint16_t nDmxStartAddress;
	uint32_t nSpeedHz = 0;

	const auto nReturnCode = Sscan::Spi(pLine, nChipSelect, aSubDeviceName, nLength, nAddress, nDmxStartAddress, nSpeedHz);

	if ((nReturnCode == Sscan::OK) && (aSubDeviceName[0] != 0) && (nLength != 0)) {
		DEBUG_PRINTF("{%.*s}:%d, nChipSelect=%d, nAddress=%d, nDmxStartAddress=%d, nSpeedHz=%d", nLength, aSubDeviceName, static_cast<int>(nLength), nChipSelect, nAddress, nDmxStartAddress, nSpeedHz);

		type subDeviceType;

		if ((subDeviceType = RDMSubDevices::GetTypeString(aSubDeviceName)) == type::UNDEFINED) {
			return;
		}

		uint32_t i;

		for (i = 0; i < m_tRDMSubDevicesParams.nCount; i++) {
			if ((m_tRDMSubDevicesParams.Entry[i].nChipSelect == static_cast<uint8_t>(nChipSelect))
					&& (m_tRDMSubDevicesParams.Entry[i].nType == subDeviceType)
					&& (m_tRDMSubDevicesParams.Entry[i].nAddress == nAddress)) {
				return;
			}
		}

		if (m_tRDMSubDevicesParams.nCount == rdm::subdevices::max) {
			return;
		}

		m_tRDMSubDevicesParams.nCount++;
		m_tRDMSubDevicesParams.Entry[i].nType = subDeviceType;
		m_tRDMSubDevicesParams.Entry[i].nChipSelect = static_cast<uint8_t>(nChipSelect);
		m_tRDMSubDevicesParams.Entry[i].nAddress = nAddress;
		m_tRDMSubDevicesParams.Entry[i].nDmxStartAddress = nDmxStartAddress;
		m_tRDMSubDevicesParams.Entry[i].nSpeedHz = nSpeedHz;
	}
}

void RDMSubDevicesParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RDMSubDevicesParams*>(p))->callbackFunction(s);
}
