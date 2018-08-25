#if defined (HAVE_I2C)
/**
 * @file sensorina219voltage.cpp
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

#if defined (__linux__)
 #include <string.h>
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "sensorina219voltage.h"
#include "rdm_e120.h"

#include "ina219.h"

static struct _device_info sDeviceInfo;

SensorINA219Voltage::SensorINA219Voltage(uint8_t nSensor, uint8_t nAddress) : RDMSensor(nSensor) {
	SetType(E120_SENS_VOLTAGE);
	SetUnit(E120_UNITS_VOLTS_DC);
	SetPrefix(E120_PREFIX_MILLI);
	SetRangeMin(-32000);
	SetRangeMax(32000);
	SetNormalMin(RDM_SENSOR_RANGE_MIN);
	SetNormalMax(RDM_SENSOR_RANGE_MAX);
	SetDescription("Voltage");

	memset(&sDeviceInfo, 0, sizeof(struct _device_info));
	sDeviceInfo.slave_address = nAddress;
}

SensorINA219Voltage::~SensorINA219Voltage(void) {
}

bool SensorINA219Voltage::Initialize(void) {
	const bool IsConnected = ina219_start(&sDeviceInfo);

#ifndef NDEBUG
	printf("%s\tIsConnected=%d\n", __FUNCTION__, (int) IsConnected);
#endif
	return IsConnected;
}

int16_t SensorINA219Voltage::GetValue(void) {
	const float bus_voltage = ina219_get_bus_voltage(&sDeviceInfo);
	const uint16_t nValue = (uint16_t) (bus_voltage * 1000);

#ifndef NDEBUG
	printf("%s\tnValue=%d\n", __FUNCTION__, (int) nValue);
#endif
	return nValue;
}
#endif
