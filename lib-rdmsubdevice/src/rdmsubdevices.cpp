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
#include <cassert>

#include "rdmsubdevices.h"
#include "rdmsubdevice.h"

#include "rdmsubdevicedummy.h"

#include "debug.h"

RDMSubDevices *RDMSubDevices::s_pThis = nullptr;

RDMSubDevices::RDMSubDevices()  {
	assert(s_pThis == nullptr);
	s_pThis = this;

#if defined(RDM_SUBDEVICES_ENABLE)
	m_pRDMSubDevice = new RDMSubDevice*[rdm::subdevices::max];
	assert(m_pRDMSubDevice != 0);

# ifndef NDEBUG
	Add(new RDMSubDeviceDummy);
# endif
#endif
}

RDMSubDevices::~RDMSubDevices() {
	for (unsigned i = 0; i < m_nCount; i++) {
		delete m_pRDMSubDevice[i];
		m_pRDMSubDevice[i] = nullptr;
	}

	delete [] m_pRDMSubDevice;

	m_nCount = 0;
}

struct TRDMSubDevicesInfo* RDMSubDevices::GetInfo(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetInfo();
}

void RDMSubDevices::GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData* pInfoData) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
	assert(pInfoData != nullptr);

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	m_pRDMSubDevice[nSubDevice - 1]->GetLabel(pInfoData);
}

void RDMSubDevices::SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));
	assert(pLabel != nullptr);

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	m_pRDMSubDevice[nSubDevice - 1]->SetLabel(pLabel,nLabelLength);
}


uint16_t RDMSubDevices::GetDmxStartAddress(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetDmxStartAddress();
}

void RDMSubDevices::SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	m_pRDMSubDevice[nSubDevice - 1]->SetDmxStartAddress(nDmxStartAddress);
}

uint16_t RDMSubDevices::GetDmxFootPrint(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetDmxFootPrint();
}

uint8_t RDMSubDevices::GetPersonalityCurrent(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCurrent();
}

void RDMSubDevices::SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	m_pRDMSubDevice[nSubDevice - 1]->SetPersonalityCurrent(nPersonality);
}

uint8_t RDMSubDevices::GetPersonalityCount(uint16_t nSubDevice) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonalityCount();
}

RDMPersonality* RDMSubDevices::GetPersonality(uint16_t nSubDevice, uint8_t nPersonality) {
	assert((nSubDevice != 0) && (nSubDevice <= m_nCount));

	assert(m_pRDMSubDevice[nSubDevice - 1] != nullptr);
	return m_pRDMSubDevice[nSubDevice - 1]->GetPersonality(nPersonality);
}

void RDMSubDevices::Start() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != nullptr) {
			m_pRDMSubDevice[i]->Start();
		}
	}

	DEBUG_EXIT
}

void RDMSubDevices::Stop() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != nullptr) {
			m_pRDMSubDevice[i]->Stop();
		}
	}

	DEBUG_ENTRY
}

void RDMSubDevices::SetData(const uint8_t* pData, uint16_t nLength) {
	for (unsigned i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != nullptr) {
			if (nLength >= m_pRDMSubDevice[i]->GetDmxStartAddress() + m_pRDMSubDevice[i]->GetDmxFootPrint() - 1) {
				m_pRDMSubDevice[i]->Data(pData, nLength);
			}
		}
	}
}

bool RDMSubDevices::GetFactoryDefaults() {
	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != nullptr) {
			if (!m_pRDMSubDevice[i]->GetFactoryDefaults()) {
				return false;
			}
		}
	}

	return true;
}

void RDMSubDevices::SetFactoryDefaults() {
	for (uint32_t i = 0; i < m_nCount; i++) {
		if (m_pRDMSubDevice[i] != nullptr) {
			m_pRDMSubDevice[i]->SetFactoryDefaults();
		}
	}
}
