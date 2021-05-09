/**
 * @file rdmsensorina219voltage.h
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

#ifndef RDMSENSORINA219VOLTAGE_H_
#define RDMSENSORINA219VOLTAGE_H_

#include <cstdint>

#include "rdmsensor.h"
#include "ina219.h"

#include "rdm_e120.h"

class RDMSensorINA219Voltage: public RDMSensor, sensor::INA219 {
public:
	RDMSensorINA219Voltage(uint8_t nSensor, uint8_t nAddress = 0) : RDMSensor(nSensor), sensor::INA219(nAddress) {
		SetType(E120_SENS_CURRENT);
		SetUnit(E120_UNITS_AMPERE_DC);
		SetPrefix(E120_PREFIX_MILLI);
		SetRangeMin(rdm::sensor::safe_range_min(sensor::ina219::voltage::RANGE_MIN));
		SetRangeMax(rdm::sensor::safe_range_max(sensor::ina219::voltage::RANGE_MAX));
		SetNormalMin(rdm::sensor::safe_range_min(sensor::ina219::voltage::RANGE_MIN));
		SetNormalMax(rdm::sensor::safe_range_max(sensor::ina219::voltage::RANGE_MAX));
		SetDescription(sensor::ina219::voltage::DESCRIPTION);
	}

	bool Initialize() override {
		return sensor::INA219::Initialize();
	}

	int16_t GetValue() override {
		return static_cast<int16_t>(sensor::INA219::GetBusVoltage() * 1000);
	}
};

#endif /* RDMSENSORINA219VOLTAGE_H_ */
