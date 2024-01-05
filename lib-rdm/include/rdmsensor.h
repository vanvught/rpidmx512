/**
 * @file rdmsensor.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSOR_H_
#define RDMSENSOR_H_

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <algorithm>

#include "debug.h"

namespace rdm {
namespace sensor {
struct Defintion {
	uint8_t sensor;
	uint8_t type;
	uint8_t unit;
	uint8_t prefix;
	int16_t range_min;
	int16_t range_max;
	int16_t normal_min;
	int16_t normal_max;
	char description[32];
	uint8_t nLength;
	uint8_t recorded_supported;
};

struct Values {
	int16_t present;
	int16_t lowest_detected;
	int16_t highest_detected;
	int16_t recorded;
	uint8_t sensor_requested;
};

static constexpr int16_t RANGE_MIN = -32768;
static constexpr int16_t RANGE_MAX = +32767;
static constexpr int16_t NORMAL_MIN = -32768;
static constexpr int16_t NORMAL_MAX = +32767;
static constexpr int16_t TEMPERATURE_ABS_ZERO = -273;

static constexpr uint8_t RECORDED_SUPPORTED = (1U << 0);
static constexpr uint8_t LOW_HIGH_DETECT = (1U << 1);

template<class T>
constexpr int16_t safe_range_max(const T &a) {
	static_assert(sizeof(int16_t) <= sizeof(T), "T");
	return (a > static_cast<T>(INT16_MAX)) ? INT16_MAX : static_cast<int16_t>(a);
}

template<class T>
constexpr int16_t safe_range_min(const T &a) {
	static_assert(sizeof(int16_t) <= sizeof(T), "T");
	return (a < static_cast<T>(INT16_MIN)) ? INT16_MIN : static_cast<int16_t>(a);
}
}  // namespace rdm
}  // namespace sensor


class RDMSensor {
public:
	RDMSensor(const uint8_t nSensor) : m_nSensor(nSensor) {
		DEBUG_ENTRY

		m_tRDMSensorDefintion.sensor = m_nSensor;
		m_tRDMSensorDefintion.recorded_supported = rdm::sensor::RECORDED_SUPPORTED | rdm::sensor::LOW_HIGH_DETECT;

		m_tRDMSensorValues.sensor_requested = m_nSensor;
		m_tRDMSensorValues.lowest_detected = rdm::sensor::RANGE_MAX;
		m_tRDMSensorValues.highest_detected = rdm::sensor::RANGE_MIN;

		DEBUG_EXIT
	}

	virtual ~RDMSensor() = default;

public:
	void SetType(const uint8_t nType) {
		m_tRDMSensorDefintion.type = nType;
	}

	void SetUnit(const uint8_t nUnit) {
		m_tRDMSensorDefintion.unit = nUnit;
	}

	void SetPrefix(const uint8_t nPrefix) {
		m_tRDMSensorDefintion.prefix = nPrefix;
	}

	void SetRangeMin(const int16_t nRangeMin) {
		m_tRDMSensorDefintion.range_min = nRangeMin;
	}

	void SetRangeMax(const int16_t nRangeMax) {
		m_tRDMSensorDefintion.range_max = nRangeMax;
	}

	void SetNormalMin(const int16_t nNormalMin) {
		m_tRDMSensorDefintion.normal_min = nNormalMin;
	}

	void SetNormalMax(const int16_t nNormalMax) {
		m_tRDMSensorDefintion.normal_max = nNormalMax;
	}

	void SetDescription(const char *pDescription) {
		DEBUG_ENTRY

		assert(pDescription != nullptr);
		uint32_t i;

		for (i = 0; i < 32 && pDescription[i] != 0; i++) {
			m_tRDMSensorDefintion.description[i] = pDescription[i];
		}

		m_tRDMSensorDefintion.nLength = static_cast<uint8_t>(i);

		DEBUG_EXIT
	}

	void Print() {
		printf("%d [%.*s]\n", m_tRDMSensorDefintion.sensor, m_tRDMSensorDefintion.nLength, m_tRDMSensorDefintion.description);
		printf(" RangeMin  %d\n", m_tRDMSensorDefintion.range_min);
		printf(" RangeMax  %d\n", m_tRDMSensorDefintion.range_max);
		printf(" NormalMin %d\n", m_tRDMSensorDefintion.normal_min);
		printf(" NormalMax %d\n", m_tRDMSensorDefintion.normal_max);
	}

	uint8_t GetSensor() const {
		return m_nSensor;
	}

	const struct rdm::sensor::Defintion* GetDefintion() {
		return &m_tRDMSensorDefintion;
	}

	const struct rdm::sensor::Values *GetValues() {
		DEBUG_ENTRY
		const auto nValue = this->GetValue();

		m_tRDMSensorValues.present = nValue;
		m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
		m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

		DEBUG_EXIT
		return &m_tRDMSensorValues;
	}

	void SetValues() {
		DEBUG_ENTRY
		const auto nValue = this->GetValue();

		m_tRDMSensorValues.present = nValue;
		m_tRDMSensorValues.lowest_detected = nValue;
		m_tRDMSensorValues.highest_detected = nValue;
		m_tRDMSensorValues.recorded = nValue;

		DEBUG_EXIT
	}

	void Record() {
		DEBUG_ENTRY
		const auto nValue = this->GetValue();

		m_tRDMSensorValues.present = nValue;
		m_tRDMSensorValues.recorded = nValue;
		m_tRDMSensorValues.lowest_detected = std::min(m_tRDMSensorValues.lowest_detected, nValue);
		m_tRDMSensorValues.highest_detected = std::max(m_tRDMSensorValues.highest_detected, nValue);

		DEBUG_EXIT
	}

	virtual bool Initialize()=0;
	virtual int16_t GetValue()=0;

private:
	uint8_t m_nSensor;
	rdm::sensor::Defintion m_tRDMSensorDefintion;
	rdm::sensor::Values m_tRDMSensorValues;
};

#endif /* RDMSENSOR_H_ */
