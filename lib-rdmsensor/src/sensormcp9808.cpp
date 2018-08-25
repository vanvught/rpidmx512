#if defined (HAVE_I2C)
/**
 * @file sensormcp9808.cpp
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

#include "sensormcp9808.h"
#include "rdm_e120.h"

#include "mcp9808.h"

static struct _device_info sDeviceInfo;

SensorMCP9808::SensorMCP9808(uint8_t nSensor, uint8_t nAddress) : RDMSensor(nSensor) {
	SetType(E120_SENS_TEMPERATURE);
	SetUnit(E120_UNITS_CENTIGRADE);
	SetPrefix(E120_PREFIX_NONE);
	SetRangeMin(-40);
	SetRangeMax(125);
	SetNormalMin(18);
	SetNormalMax(40);
	SetDescription("Ambient Temperature");

	memset(&sDeviceInfo, 0, sizeof(struct _device_info));
	sDeviceInfo.slave_address = nAddress;
}

SensorMCP9808::~SensorMCP9808(void) {
}

bool SensorMCP9808::Initialize(void) {
	const bool IsConnected = mcp9808_start(&sDeviceInfo);

#ifndef NDEBUG
	printf("%s\tIsConnected=%d\n", __FUNCTION__, (int) IsConnected);
#endif
	return IsConnected;
}

int16_t SensorMCP9808::GetValue(void) {
	const uint16_t nValue = (uint16_t) mcp9808_get_temperature(&sDeviceInfo);

#ifndef NDEBUG
	printf("%s\tnValue=%d\n", __FUNCTION__, (int) nValue);
#endif
	return nValue;
}
#endif
