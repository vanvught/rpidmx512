/**
 * @file gpioparams.cpp
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

#include "dmxgpioparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "util.h"

#include "gpio.h"

#define DATA_DIRECTION_MASK			(1 << 0)
#define DATA_DIRECTION_OUT_A_MASK	(1 << 1)
#define DATA_DIRECTION_OUT_B_MASK	(1 << 2)
#define DATA_DIRECTION_OUT_C_MASK	(1 << 3)
#define DATA_DIRECTION_OUT_D_MASK	(1 << 4)

static const char PARAMS_FILE_NAME[] ALIGNED = "gpio.txt";
static const char PARAMS_DATA_DIRECTION[] ALIGNED = "data_direction";
static const char PARAMS_DATA_DIRECTION_OUT_A[] ALIGNED = "data_direction_out_a";
static const char PARAMS_DATA_DIRECTION_OUT_B[] ALIGNED = "data_direction_out_b";
static const char PARAMS_DATA_DIRECTION_OUT_C[] ALIGNED = "data_direction_out_c";
static const char PARAMS_DATA_DIRECTION_OUT_D[] ALIGNED = "data_direction_out_d";

DmxGpioParams::DmxGpioParams(void):
		m_nSetList(0),
		m_nDmxDataDirection(GPIO_DMX_DATA_DIRECTION)
{
#if defined(H3)
 #if defined(ORANGE_PI_ONE)
	m_nDmxDataDirectionOut[0] = GPIO_DMX_DATA_DIRECTION_OUT_D;
	m_nDmxDataDirectionOut[1] = GPIO_DMX_DATA_DIRECTION_OUT_A;
	m_nDmxDataDirectionOut[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	m_nDmxDataDirectionOut[3] = GPIO_DMX_DATA_DIRECTION_OUT_C;
 #else
	m_nDmxDataDirectionOut[1] = GPIO_DMX_DATA_DIRECTION_OUT_C;
	m_nDmxDataDirectionOut[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
 #endif
#else
	m_nDmxDataDirectionOut[0] = GPIO_DMX_DATA_DIRECTION;
#endif

	ReadConfigFile configfile(DmxGpioParams::staticCallbackFunction, this);
	configfile.Read(PARAMS_FILE_NAME);
}

DmxGpioParams::~DmxGpioParams(void) {
}

bool DmxGpioParams::Load(void) {
	m_nSetList = 0;

	ReadConfigFile configfile(DmxGpioParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void DmxGpioParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;

	if (Sscan::Uint8(pLine, PARAMS_DATA_DIRECTION, &value8) == SSCAN_OK) {
		if (value8 < 32) {
			m_nDmxDataDirection = value8;
			m_nSetList |= DATA_DIRECTION_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_DATA_DIRECTION_OUT_A, &value8) == SSCAN_OK) {
		m_nDmxDataDirectionOut[0] = value8;
		m_nSetList |= DATA_DIRECTION_OUT_A_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_DATA_DIRECTION_OUT_B, &value8) == SSCAN_OK) {
		m_nDmxDataDirectionOut[1] = value8;
		m_nSetList |= DATA_DIRECTION_OUT_B_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_DATA_DIRECTION_OUT_C, &value8) == SSCAN_OK) {
		m_nDmxDataDirectionOut[2] = value8;
		m_nSetList |= DATA_DIRECTION_OUT_C_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_DATA_DIRECTION_OUT_D, &value8) == SSCAN_OK) {
		m_nDmxDataDirectionOut[3] = value8;
		m_nSetList |= DATA_DIRECTION_OUT_D_MASK;
		return;
	}
}

uint8_t DmxGpioParams::GetDataDirection(bool &bIsSet) const {
	bIsSet = isMaskSet(DATA_DIRECTION_MASK);
	return m_nDmxDataDirection;
}

uint8_t DmxGpioParams::GetDataDirection(bool &bIsSet, uint8_t nUart) const {
	assert(nUart < DMX_MAX_OUT);

	switch (nUart) {
	case 0:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_A_MASK);
		break;
	case 1:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_B_MASK);
		break;
	case 2:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_C_MASK);
		break;
	case 3:
		bIsSet = isMaskSet(DATA_DIRECTION_OUT_D_MASK);
		break;
	default:
		break;
	}

	return m_nDmxDataDirectionOut[nUart];
}

void DmxGpioParams::Dump(void) {
#ifndef NDEBUG
	if (m_nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(DATA_DIRECTION_MASK)) {
		printf(" %s=%d\n", PARAMS_DATA_DIRECTION, m_nDmxDataDirection);
	}

	if(isMaskSet(DATA_DIRECTION_OUT_A_MASK)) {
		printf(" %s=%d\n", PARAMS_DATA_DIRECTION_OUT_A, m_nDmxDataDirectionOut[0]);
	}

	if(isMaskSet(DATA_DIRECTION_OUT_B_MASK)) {
		printf(" %s=%d\n", PARAMS_DATA_DIRECTION_OUT_B, m_nDmxDataDirectionOut[1]);
	}

	if(isMaskSet(DATA_DIRECTION_OUT_C_MASK)) {
		printf(" %s=%d\n", PARAMS_DATA_DIRECTION_OUT_C, m_nDmxDataDirectionOut[2]);
	}

	if(isMaskSet(DATA_DIRECTION_OUT_D_MASK)) {
		printf(" %s=%d\n", PARAMS_DATA_DIRECTION_OUT_D, m_nDmxDataDirectionOut[3]);
	}
#endif
}

void DmxGpioParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((DmxGpioParams *) p)->callbackFunction(s);
}

bool DmxGpioParams::isMaskSet(uint32_t nMask) const {
	return (m_nSetList & nMask) == nMask;
}
