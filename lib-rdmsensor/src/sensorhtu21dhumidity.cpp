#if defined (HAVE_I2C)
/**
 * @file sensorhtu21dhumidity.h
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

#include "sensorhtu21dhumidity.h"
#include "rdm_e120.h"

#include "htu21d.h"

static struct _device_info sDeviceInfo;

SensorHTU21DHumidity::SensorHTU21DHumidity(uint8_t nSensor, uint8_t nAddress): RDMSensor(nSensor) {
	SetType(E120_SENS_HUMIDITY);
	SetUnit(E120_UNITS_NONE);
	SetPrefix(E120_PREFIX_NONE);
	SetRangeMin(0);
	SetRangeMax(100);
	SetNormalMin(RDM_SENSOR_RANGE_MIN);
	SetNormalMax(RDM_SENSOR_RANGE_MAX);
	SetDescription("Relative Humidity");

	memset(&sDeviceInfo, 0, sizeof(struct _device_info));
	sDeviceInfo.slave_address = nAddress;
}

SensorHTU21DHumidity::~SensorHTU21DHumidity(void) {
}

bool SensorHTU21DHumidity::Initialize(void) {
	const bool IsConnected = htu21d_start(&sDeviceInfo);

#ifndef NDEBUG
	printf("%s\tIsConnected=%d\n", __FUNCTION__, (int) IsConnected);
#endif
	return IsConnected;
}

int16_t SensorHTU21DHumidity::GetValue(void) {
	const int16_t nValue = (int16_t) htu21d_get_humidity(&sDeviceInfo);

#ifndef NDEBUG
	printf("%s\tnValue=%d\n", __FUNCTION__, (int) nValue);
#endif
	return nValue;
}
#endif
