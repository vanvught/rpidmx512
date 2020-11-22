/**
 * @file rdmsensors.cpp
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

#include "rdmsensors.h"

#include "debug.h"

#if defined (RDMSENSOR_CPU_ENABLE)
 #include "cputemperature.h"
#endif

RDMSensors *RDMSensors::s_pThis = nullptr;

RDMSensors::RDMSensors() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

#if defined (RDM_SENSORS_ENABLE) || defined (RDMSENSOR_CPU_ENABLE)
	m_pRDMSensor = new RDMSensor*[rdm::sensors::max];
	assert(m_pRDMSensor != nullptr);

# if defined (RDMSENSOR_CPU_ENABLE)
	Add(new CpuTemperature(m_nCount));
# endif
#endif
	DEBUG_EXIT
}

RDMSensors::~RDMSensors() {
	for (unsigned i = 0; i < m_nCount; i++) {
		if (m_pRDMSensor[i] != nullptr) {
			delete m_pRDMSensor[i];
			m_pRDMSensor[i] = nullptr;
		}
	}

	delete [] m_pRDMSensor;
}

const struct TRDMSensorDefintion* RDMSensors::GetDefintion(uint8_t nSensor) {
	assert(nSensor < m_nCount);

	assert(m_pRDMSensor[nSensor] != nullptr);
	return m_pRDMSensor[nSensor]->GetDefintion();
}

const struct TRDMSensorValues* RDMSensors::GetValues(uint8_t nSensor) {
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
