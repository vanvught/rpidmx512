/**
 * @file rdmsensorina219current.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSORINA219CURRENT_H_
#define RDMSENSORINA219CURRENT_H_

#include <cstdint>

#include "rdmsensor.h"
#include "ina219.h"

#include "rdm_e120.h"

class RDMSensorINA219Current: public RDMSensor, sensor::INA219 {
public:
	RDMSensorINA219Current(uint8_t nSensor, uint8_t address = 0) : RDMSensor(nSensor), sensor::INA219(address) {
		SetType(E120_SENS_CURRENT);
		SetUnit(E120_UNITS_AMPERE_DC);
		SetPrefix(E120_PREFIX_MILLI);
		SetRangeMin(rdm::sensor::SafeRangeMin(sensor::ina219::current::RANGE_MIN));
		SetRangeMax(rdm::sensor::SafeRangeMax(sensor::ina219::current::RANGE_MAX));
		SetNormalMin(rdm::sensor::SafeRangeMin(sensor::ina219::current::RANGE_MIN));
		SetNormalMax(rdm::sensor::SafeRangeMax(sensor::ina219::current::RANGE_MAX));
		SetDescription(sensor::ina219::current::DESCRIPTION);
	}

	bool Initialize() override {
		return sensor::INA219::Initialize();
	}

	int16_t GetValue() override {
		return static_cast<int16_t>(sensor::INA219::GetShuntCurrent() * 1000);
	}
};

#endif /* RDMSENSORINA219CURRENT_H_ */
