/**
 * @file rdmsensor.cpp
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

#include <algorithm>
#include <cstdint>
#include <cassert>

#include "rdmsensor.h"

#include "debug.h"

static constexpr uint8_t RDM_SENSOR_RECORDED_SUPPORTED = (1U << 0);
static constexpr uint8_t RDM_SENSOR_LOW_HIGH_DETECT = (1U << 1);

RDMSensor::RDMSensor(uint8_t nSensor) : m_nSensor(nSensor) {
	DEBUG_ENTRY

	m_tRDMSensorDefintion.sensor = m_nSensor;
	m_tRDMSensorDefintion.recorded_supported = RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT;

	m_tRDMSensorValues.sensor_requested = m_nSensor;
	m_tRDMSensorValues.lowest_detected = rdm::sensor::RANGE_MAX;
	m_tRDMSensorValues.highest_detected = rdm::sensor::RANGE_MIN;

	DEBUG_EXIT
}

void RDMSensor::SetDescription(const char *pDescription) {
	DEBUG_ENTRY

	assert(pDescription != nullptr);
	uint32_t i;

	for (i = 0; i < 32 && pDescription[i] != 0; i++) {
		m_tRDMSensorDefintion.description[i] = pDescription[i];
	}

	m_tRDMSensorDefintion.nLength = static_cast<uint8_t>(i);

	DEBUG_EXIT
}

const struct rdm::sensor::Values* RDMSensor::GetValues() {
	DEBUG_ENTRY

	const auto nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
	m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

	DEBUG_EXIT

	return &m_tRDMSensorValues;
}

void RDMSensor::SetValues() {
	DEBUG_ENTRY

	const auto nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.lowest_detected = nValue;
	m_tRDMSensorValues.highest_detected = nValue;
	m_tRDMSensorValues.recorded = nValue;

	DEBUG_EXIT
}

void RDMSensor::Record() {
	DEBUG_ENTRY

	const auto nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.recorded = nValue;
	m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
	m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

	DEBUG_EXIT
}
