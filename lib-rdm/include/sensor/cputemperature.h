/**
 * @file cputemperature.h
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

#ifndef CPUTEMPERATURE_H_
#define CPUTEMPERATURE_H_

#include <cstdint>

#include "rdmsensor.h"
#include "rdm_e120.h"
#include "hardware.h"

#include "debug.h"

class CpuTemperature final: public RDMSensor {
public:
	CpuTemperature(uint8_t nSensor): RDMSensor(nSensor) {
		SetType(E120_SENS_TEMPERATURE);
		SetUnit(E120_UNITS_CENTIGRADE);
		SetPrefix(E120_PREFIX_NONE);
		SetRangeMin(static_cast<int16_t>(Hardware::Get()->GetCoreTemperatureMin()));
		SetRangeMax(static_cast<int16_t>(Hardware::Get()->GetCoreTemperatureMax()));
		SetNormalMin(static_cast<int16_t>(Hardware::Get()->GetCoreTemperatureMin()));
		SetNormalMax(static_cast<int16_t>(Hardware::Get()->GetCoreTemperatureMax()));
		SetDescription("CPU");
	}

	bool Initialize() override {
		DEBUG_ENTRY
	#if defined (__APPLE__)
		DEBUG_EXIT
		return false;
	#else
		DEBUG_EXIT
		return true;
	#endif
	}

	int16_t GetValue() override {
		const auto nValue = static_cast<int16_t>(Hardware::Get()->GetCoreTemperature());
		DEBUG_PRINTF("nValue=%d", nValue);
		return nValue;
	}
};

#endif /* CPUTEMPERATURE_H_ */
