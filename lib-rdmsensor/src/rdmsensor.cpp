/**
 * @file rdmsensor.cpp
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

#include <algorithm>
#include <stdint.h>
#include <cassert>

#include "rdmsensor.h"

#include <debug.h>

static constexpr uint8_t RDM_SENSOR_RECORDED_SUPPORTED = (1U << 0);
static constexpr uint8_t RDM_SENSOR_LOW_HIGH_DETECT = (1U << 1);

RDMSensor::RDMSensor(uint8_t nSensor) : m_nSensor(nSensor) {
	DEBUG1_ENTRY

	m_tRDMSensorDefintion.sensor = m_nSensor;
	m_tRDMSensorDefintion.recorded_supported = RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT;

	m_tRDMSensorValues.sensor_requested = m_nSensor;
	m_tRDMSensorValues.lowest_detected = RDM_SENSOR_RANGE_MAX;
	m_tRDMSensorValues.highest_detected = RDM_SENSOR_RANGE_MIN;

	DEBUG1_EXIT
}

void RDMSensor::SetDescription(const char *pDescription) {
	DEBUG1_ENTRY

	assert(pDescription != nullptr);
	uint32_t i;

	for (i = 0; i < 32 && pDescription[i] != 0; i++) {
		m_tRDMSensorDefintion.description[i] = pDescription[i];
	}

	m_tRDMSensorDefintion.len = i;

	DEBUG1_EXIT
}

const struct TRDMSensorValues* RDMSensor::GetValues() {
	DEBUG1_ENTRY

	const int16_t nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
	m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

	DEBUG1_EXIT

	return &m_tRDMSensorValues;
}

void RDMSensor::SetValues() {
	DEBUG1_ENTRY

	const int16_t nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.lowest_detected = nValue;
	m_tRDMSensorValues.highest_detected = nValue;
	m_tRDMSensorValues.recorded = nValue;

	DEBUG1_EXIT
}

void RDMSensor::Record() {
	DEBUG1_ENTRY

	const int16_t nValue = this->GetValue();

	m_tRDMSensorValues.present = nValue;
	m_tRDMSensorValues.recorded = nValue;
	m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
	m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

	DEBUG1_EXIT
}
