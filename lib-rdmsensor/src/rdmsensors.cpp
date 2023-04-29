/**
 * @file rdmsensors.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "rdmsensors.h"

#include "debug.h"

#if defined (RDMSENSOR_CPU_ENABLE)
# include "cputemperature.h"
#endif

RDMSensors *RDMSensors::s_pThis = nullptr;

RDMSensors::RDMSensors() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

#if defined (RDM_SENSORS_ENABLE) || defined (RDMSENSOR_CPU_ENABLE)
	m_pRDMSensor = new RDMSensor*[rdm::sensors::MAX];
	assert(m_pRDMSensor != nullptr);

# if defined (RDMSENSOR_CPU_ENABLE)
	Add(new CpuTemperature(m_nCount));
# endif
#endif
	DEBUG_EXIT
}

RDMSensors::~RDMSensors() {
	DEBUG_ENTRY

	for (unsigned i = 0; i < m_nCount; i++) {
		if (m_pRDMSensor[i] != nullptr) {
			delete m_pRDMSensor[i];
			m_pRDMSensor[i] = nullptr;
		}
	}

	delete [] m_pRDMSensor;

	DEBUG_EXIT
}

const struct rdm::sensor::Defintion* RDMSensors::GetDefintion(uint8_t nSensor) {
	assert(nSensor < m_nCount);

	assert(m_pRDMSensor[nSensor] != nullptr);
	return m_pRDMSensor[nSensor]->GetDefintion();
}

const struct rdm::sensor::Values* RDMSensors::GetValues(uint8_t nSensor) {
	assert(nSensor < m_nCount);

	assert(m_pRDMSensor[nSensor] != nullptr);
	return m_pRDMSensor[nSensor]->GetValues();
}

void RDMSensors::SetValues(uint8_t nSensor) {
	if (nSensor == 0xFF) {
		for (uint32_t i = 0; i < m_nCount; i++) {
			m_pRDMSensor[i]->SetValues();
		}
	} else {
		m_pRDMSensor[nSensor]->SetValues();
	}
}

void RDMSensors::SetRecord(uint8_t nSensor) {
	if (nSensor == 0xFF) {
		for (uint32_t i = 0; i < m_nCount; i++) {
			m_pRDMSensor[i]->Record();
		}
	} else {
		m_pRDMSensor[nSensor]->Record();
	}
}

bool RDMSensors::Add(RDMSensor *pRDMSensor) {
	DEBUG_ENTRY

	assert(m_pRDMSensor != nullptr);

	if (m_pRDMSensor == nullptr) {
		DEBUG_EXIT
		return false;
	}

	if (m_nCount == rdm::sensors::MAX) {
		DEBUG_EXIT
		return false;
	}

	assert(pRDMSensor != nullptr);
	m_pRDMSensor[m_nCount++] = pRDMSensor;

	DEBUG_PRINTF("m_nCount=%u", m_nCount);
	DEBUG_EXIT
	return true;
}

// Static

const char* RDMSensors::GetTypeString(rdm::sensors::Types type) {
	if (type < rdm::sensors::Types::UNDEFINED) {
		return RDMSensorsConst::TYPE[static_cast<uint32_t>(type)];
	}

	return "Unknown";
}

rdm::sensors::Types RDMSensors::GetTypeString(const char *pValue) {
	assert(pValue != nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(rdm::sensors::Types::UNDEFINED); i++) {
		if (strcasecmp(pValue, RDMSensorsConst::TYPE[i]) == 0) {
			return static_cast<rdm::sensors::Types>(i);
		}
	}

	return rdm::sensors::Types::UNDEFINED;
}
