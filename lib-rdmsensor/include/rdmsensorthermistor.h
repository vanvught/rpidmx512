/**
 * @file rdmsensorthermistor.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMSENSORTHERMISTOR_H_
#define RDMSENSORTHERMISTOR_H_

#include <cstdint>

#include "rdmsensor.h"
#include "rdmsensorsstore.h"
#include "rdm_e120.h"

#include "mcp3424.h"
#include "thermistor.h"

#include "debug.h"

class RDMSensorThermistor final: public RDMSensor, MCP3424 {
public:
	RDMSensorThermistor(uint8_t nSensor, uint8_t nAddress = 0, uint8_t nChannel = 0, int32_t nCalibration = 0) :
	RDMSensor(nSensor),
	MCP3424(nAddress),
	m_nCalibration(nCalibration),
	m_nChannel(nChannel)
	{
		DEBUG_ENTRY
		DEBUG_PRINTF("nSensor=%u, nAddress=0x%.2x, nChannel=%u, nCalibration=%d", nSensor, nAddress, nChannel, nCalibration);

		SetType(E120_SENS_TEMPERATURE);
		SetUnit(E120_UNITS_CENTIGRADE);
		SetPrefix(E120_PREFIX_NONE);
		SetRangeMin(rdm::sensor::safe_range_min(sensor::thermistor::RANGE_MIN));
		SetRangeMax(rdm::sensor::safe_range_max(sensor::thermistor::RANGE_MAX));
		SetNormalMin(rdm::sensor::safe_range_min(sensor::thermistor::RANGE_MIN));
		SetNormalMax(rdm::sensor::safe_range_max(sensor::thermistor::RANGE_MAX));
		SetDescription(sensor::thermistor::DESCRIPTION);

		DEBUG_EXIT
	}

	bool Initialize() override {
		return MCP3424::IsConnected();
	}

	bool Calibrate(float f) {
		const auto iCalibrate = static_cast<int32_t>(f * 10);
		uint32_t nResistor;
		const auto iMeasure = static_cast<int32_t>(GetValue(nResistor) * 10);

		DEBUG_PRINTF("iCalibrate=%d, iMeasure=%d", iCalibrate, iMeasure);

		if (iCalibrate == iMeasure) {
			return true;
		}

		int32_t Offset = 10;

		if (iCalibrate > iMeasure) {
			Offset = -10;
		}

		for (int32_t i = 1; i < 128; i++) {
			m_nCalibration = i * Offset;
			const auto iMeasure = static_cast<int32_t>(GetValue(nResistor) * 10);
			DEBUG_PRINTF("iCalibrate=%d, iMeasure=%d, m_nOffset=%d, nResistor=%u", iCalibrate, iMeasure, m_nCalibration, nResistor);
			if (iCalibrate == iMeasure) {
				RDMSensorsStore::SaveCalibration(RDMSensor::GetSensor(), m_nCalibration);
				return true;
			}
		}

		return false;
	}

	void ResetCalibration() {
		m_nCalibration = 0;
		RDMSensorsStore::SaveCalibration(RDMSensor::GetSensor(), m_nCalibration);
	}

	int32_t GetCalibration() const {
		return m_nCalibration;
	}

	float GetValue(uint32_t &nResistor) {
		double sum = 0;
		for (uint32_t i = 0; i < 4; i++) {
			const auto v = MCP3424::GetVoltage(m_nChannel);
			sum += v;
		}
		const auto v = sum / 4;
		const auto r = resistor(v);
		const auto t = sensor::thermistor::temperature(r);
		DEBUG_PRINTF("v=%1.3f, r=%u, t=%3.1f", v, r, t);
		nResistor = r;
		return t;
	}

	int16_t GetValue() override {
		uint32_t nResistor;
		return static_cast<int16_t>(GetValue(nResistor));
	}

private:
	int32_t m_nCalibration;
	uint8_t m_nChannel;

	/*
	 * The R values are based on:
	 * https://www.abelectronics.co.uk/p/69/adc-pi-raspberry-pi-analogue-to-digital-converter
	 */
	static constexpr int32_t R_GND = 6800;		// 6K8
	static constexpr int32_t R_HIGH = 10000;	// 10K

	uint32_t resistor(const double vin) {
		const double d = (5 * R_GND) / vin;
		const auto r = static_cast<int32_t>(d) - R_GND - R_HIGH + m_nCalibration;
		return static_cast<uint32_t>(r);
	}
};

#endif /* RDMSENSORTHERMISTOR_H_ */
