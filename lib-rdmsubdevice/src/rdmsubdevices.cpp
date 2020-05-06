/**
 * @file rdmsubdevices.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <assert.h>

#include "readconfigfile.h"
#include "sscan.h"

#include "debug.h"

#include "rdmsubdevices.h"
#include "rdmsubdevice.h"

#include "rdmsubdevicedummy.h"

#if defined(BARE_METAL) && !( defined (ARTNET_NODE) && defined(RDM_RESPONDER) )
 #define RDM_SUBDEVICES_ENABLE
#endif

#if defined (RDMNET_LLRP_ONLY)
 #undef RDM_SUBDEVICES_ENABLE
#endif

#if defined(RDM_SUBDEVICES_ENABLE)
#include "devicesparamsconst.h"
//
#include "rdmsubdevicebw7fets.h"
#include "rdmsubdevicebwdimmer.h"
#include "rdmsubdevicebwdio.h"
#include "rdmsubdevicebwlcd.h"
#include "rdmsubdevicebwrelay.h"
//
#include "rdmsubdevicemcp23s08.h"
#include "rdmsubdevicemcp23s17.h"
#include "rdmsubdevicemcp4822.h"
#include "rdmsubdevicemcp4902.h"
#endif

#define RDM_SUBDEVICES_MAX	8

RDMSubDevices *RDMSubDevices::s_pThis = 0;

RDMSubDevices::RDMSubDevices(void) : m_nCount(0) {
	s_pThis = this;

	m_pRDMSubDevice = new RDMSubDevice*[RDM_SUBDEVICES_MAX];
	assert(m_pRDMSubDevice != 0);
}

RDMSubDevices::~RDMSubDevices(void) {
	for (unsigned i = 0; i < m_nCount; i++) {
		delete m_pRDMSubDevice[i];
		m_pRDMSubDevice[i] = 0;
	}

	delete [] m_pRDMSubDevice;

	m_nCount = 0;
}

void RDMSubDevices::Init(void) {
#ifndef NDEBUG
	Add(new RDMSubDeviceDummy);
#endif

#if defined(RDM_SUBDEVICES_ENABLE)
	ReadConfigFile configfile(RDMSubDevices::staticCallbackFunction, this);
	configfile.Read(DevicesParamsConst::FILE_NAME);
#endif

	DEBUG_PRINTF("SubDevices added: %d", m_nCount);
}

bool RDMSubDevices::Add(RDMSubDevice* pRDMSubDevice) {
	if(m_nCount == RDM_SUBDEVICES_MAX) {
		return false;
	}

	assert(pRDMSubDevice != 0);

	if (!pRDMSubDevice->Initialize()) {
		delete pRDMSubDevice;
		pRDMSubDevice = 0;
		return false;
	}

	m_pRDMSubDevice[m_nCount++] = pRDMSubDevice;

	return true;
}

uint16_t RDMSubDevices::GetCount(void) const {
	return m_nCount;
}

struct TRDMSubDevicesInfo* RDMSubDevices::GetInfo(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetInfo();
}

void RDMSubDevices::GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData* pInfoData) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
	assert(pInfoData != 0);

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	m_pRDMSubDevice[nSubDevice - 1]->GetLabel(pInfoData);
}

void RDMSubDevices::SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
	assert(pLabel != 0);

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	m_pRDMSubDevice[nSubDevice - 1]->SetLabel(pLabel,nLabelLength);
}


uint16_t RDMSubDevices::GetDmxStartAddress(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetDmxStartAddress();
}

void RDMSubDevices::SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	m_pRDMSubDevice[nSubDevice - 1]->SetDmxStartAddress(nDmxStartAddress);
}

uint16_t RDMSubDevices::GetDmxFootPrint(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetDmxFootPrint();
}

uint8_t RDMSubDevices::GetPersonalityCurrent(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCurrent();
}

void RDMSubDevices::SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	m_pRDMSubDevice[nSubDevice - 1]->SetPersonalityCurrent(nPersonality);
}

uint8_t RDMSubDevices::GetPersonalityCount(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCount();
}

RDMPersonality* RDMSubDevices::GetPersonality(uint16_t nSubDevice, uint8_t nPersonality) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != 0);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonality(nPersonality);
}

void RDMSubDevices::Start(void) {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != 0) {
			m_pRDMSubDevice[i]->Start();
		}
	}

	DEBUG_EXIT
}

void RDMSubDevices::Stop(void) {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != 0) {
			m_pRDMSubDevice[i]->Stop();
		}
	}

	DEBUG_ENTRY
}

void RDMSubDevices::SetData(const uint8_t* pData, uint16_t nLength) {
	for (unsigned i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != 0) {
			if (nLength >= m_pRDMSubDevice[i]->GetDmxStartAddress() + m_pRDMSubDevice[i]->GetDmxFootPrint() - 1) {
				m_pRDMSubDevice[i]->Data(pData, nLength);
			}
		}
	}
}

bool RDMSubDevices::GetFactoryDefaults(void) {
	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != 0) {
			if (!m_pRDMSubDevice[i]->GetFactoryDefaults()) {
				return false;
			}
		}
	}

	return true;
}

void RDMSubDevices::SetFactoryDefaults(void) {
	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != 0) {
			m_pRDMSubDevice[i]->SetFactoryDefaults();
		}
	}
}

void RDMSubDevices::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<RDMSubDevices*>(p))->callbackFunction(s);
}

void RDMSubDevices::callbackFunction(const char* pLine) {
#if defined(RDM_SUBDEVICES_ENABLE)
	assert(pLine != 0);
	int nReturnCode;
	char aDeviceName[65];
	uint8_t nLength = sizeof(aDeviceName) - 1;
	char nChipSselect = -1;
	uint8_t nSlaveAddress = 0;
	uint16_t nDmxStartAddress = 0;
	uint32_t nSpiSpeed = 0;

	memset(aDeviceName, 0, sizeof(aDeviceName));

	nReturnCode = Sscan::Spi(pLine, &nChipSselect, aDeviceName, &nLength, &nSlaveAddress, &nDmxStartAddress, &nSpiSpeed);

	if ((nReturnCode == SSCAN_OK) && (nLength != 0)) {

		DEBUG_PRINTF("%s [%d] SPI%d %x %d %ld", aDeviceName, static_cast<int>(nLength), static_cast<int>(nChipSselect), static_cast<int>(nSlaveAddress), static_cast<int>(nDmxStartAddress), static_cast<long int>(nSpiSpeed));

		if ((nChipSselect < 0) || (nChipSselect > 2) || (nDmxStartAddress == 0) || (nDmxStartAddress > 512) ) {
			DEBUG_EXIT
			return;
		}

		if (memcmp(aDeviceName, "bw_spi_7fets", 12) == 0) {
			Add(new RDMSubDeviceBw7fets(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "bw_spi_dimmer", 13) == 0) {
			Add(new RDMSubDeviceBwDimmer(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "bw_spi_dio", 10) == 0) {
			Add(new RDMSubDeviceBwDio(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "bw_spi_lcd", 10) == 0) {
			Add(new RDMSubDeviceBwLcd(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "bw_spi_relay", 12) == 0) {
			Add(new RDMSubDeviceBwRelay(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "mcp23s08", 8) == 0) {
			Add(new RDMSubDeviceMCP23S08(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "mcp23s17", 8) == 0) {
			Add(new RDMSubDeviceMCP23S17(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "mcp4822", 7) == 0) {
			Add(new RDMSubDeviceMCP4822(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		} else if (memcmp(aDeviceName, "mcp4902", 7) == 0) {
			Add(new RDMSubDeviceMCP4902(nDmxStartAddress, nChipSselect, nSlaveAddress, nSpiSpeed));
		}
	}
#endif
}
