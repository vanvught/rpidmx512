/**
 * @file rdmsensor.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "rdmsensor.h"

#include <debug.h>

#ifndef MAX
#  define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#  define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#define RDM_SENSOR_RECORDED_SUPPORTED		(1 << 0)	///<
#define RDM_SENSOR_LOW_HIGH_DETECT			(1 << 1)	///<

RDMSensor::RDMSensor(uint8_t nSensor) : m_nSensor(nSensor) {
	DEBUG1_ENTRY

	m_tRDMSensorDefintion.sensor = m_nSensor;
	m_tRDMSensorDefintion.recorded_supported = RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT;

	m_tRDMSensorValues.sensor_requested = m_nSensor;
	m_tRDMSensorValues.lowest_detected = RDM_SENSOR_RANGE_MAX;
	m_tRDMSensorValues.highest_detected = RDM_SENSOR_RANGE_MIN;

	DEBUG1_EXIT
}

RDMSensor::~RDMSensor(void) {
}

void RDMSensor::SetType(uint8_t nType) {
	m_tRDMSensorDefintion.type = nType;
}

void RDMSensor::SetUnit(uint8_t nUnit) {
	m_tRDMSensorDefintion.unit = nUnit;
}

void RDMSensor::SetPrefix(uint8_t nPrefix) {
	m_tRDMSensorDefintion.prefix = nPrefix;
}

void RDMSensor::SetRangeMin(uint16_t nRangeMin) {
	m_tRDMSensorDefintion.range_min = nRangeMin;
}

void RDMSensor::SetRangeMax(uint16_t nRangeMax) {
	m_tRDMSensorDefintion.range_max = nRangeMax;
}

void RDMSensor::SetNormalMin(uint16_t nNormalMin) {
	m_tRDMSensorDefintion.normal_min = nNormalMin;
}

void RDMSensor::SetNormalMax(uint16_t nNormalMax) {
	m_tRDMSensorDefintion.normal_max = nNormalMax;
}

void RDMSensor::SetDescription(const char* pDescription) {
	assert(pDescription !=0 );
	uint8_t i;

	for (i = 0; i < 32 && pDescription[i] != 0; i++) {
		m_tRDMSensorDefintion.description[i] = pDescription[i];
	}

	m_tRDMSensorDefintion.len = i;
}

const struct TRDMSensorValues* RDMSensor::GetValues(void) {
	DEBUG1_ENTRY

	const uint16_t value = this->GetValue();

	m_tRDMSensorValues.present = value;
	m_tRDMSensorValues.lowest_detected = MIN(m_tRDMSensorValues.lowest_detected, value);
	m_tRDMSensorValues.highest_detected = MAX(m_tRDMSensorValues.highest_detected, value);

	DEBUG1_EXIT

	return &m_tRDMSensorValues;
}

void RDMSensor::SetValues(void) {
	DEBUG1_ENTRY

	const uint16_t value = this->GetValue();

	m_tRDMSensorValues.present = value;
	m_tRDMSensorValues.lowest_detected = value;
	m_tRDMSensorValues.highest_detected = value;
	m_tRDMSensorValues.recorded = value;

	DEBUG1_EXIT
}

void RDMSensor::Record(void) {
	DEBUG1_ENTRY

	const uint16_t value = this->GetValue();

	m_tRDMSensorValues.present = value;
	m_tRDMSensorValues.recorded = value;
	m_tRDMSensorValues.lowest_detected = MIN(m_tRDMSensorValues.lowest_detected, value);
	m_tRDMSensorValues.highest_detected = MAX(m_tRDMSensorValues.highest_detected, value);

	DEBUG1_EXIT
}

