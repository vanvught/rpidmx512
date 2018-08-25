/**
 * @file cputemperature.cpp
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

#include "cputemperature.h"
#include "rdm_e120.h"
#include "hardware.h"

CpuTemperature::CpuTemperature(uint8_t nSensor): RDMSensor(nSensor) {
	SetType(E120_SENS_TEMPERATURE);
	SetUnit(E120_UNITS_CENTIGRADE);
	SetPrefix(E120_PREFIX_NONE);
	SetRangeMin(RDM_SENSOR_TEMPERATURE_ABS_ZERO);
	SetRangeMax(RDM_SENSOR_RANGE_MAX);
	SetNormalMin(RDM_SENSOR_TEMPERATURE_ABS_ZERO);
	SetNormalMax((int16_t) Hardware::Get()->GetCoreTemperatureMax());
	SetDescription("CPU");
}

CpuTemperature::~CpuTemperature(void) {
}

bool CpuTemperature::Initialize(void) {
#if defined (__CYGWIN__) || defined (__APPLE__)
	return false;
#else
	return true;
#endif
}

int16_t CpuTemperature::GetValue(void) {
	const int16_t nValue = (int16_t) Hardware::Get()->GetCoreTemperature();

#ifndef NDEBUG
	printf("%s\tnValue=%d\n", __FUNCTION__, (int) nValue);
#endif
	return nValue;
}
