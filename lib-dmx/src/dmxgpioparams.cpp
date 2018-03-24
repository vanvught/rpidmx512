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

#define DATA_DIRECTION_MASK	1<<0

static const char PARAMS_FILE_NAME[] ALIGNED = "gpio.txt";
static const char PARAMS_DATA_DIRECTION[] ALIGNED = "data_direction";

DmxGpioParams::DmxGpioParams(void):
		m_nSetList(0),
		m_nDmxDataDirection(GPIO_DMX_DATA_DIRECTION)
{
	ReadConfigFile configfile(DmxGpioParams::staticCallbackFunction, this);
	configfile.Read(PARAMS_FILE_NAME);
}

DmxGpioParams::~DmxGpioParams(void) {
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
}

uint8_t DmxGpioParams::GetDataDirection(bool &isSet) const {
	isSet = IsMaskSet(DATA_DIRECTION_MASK);
	return m_nDmxDataDirection;
}

void DmxGpioParams::Dump(void) {
#ifndef NDEBUG
	if (m_nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if(IsMaskSet(DATA_DIRECTION_MASK)) {
		printf("%s=%d\n", PARAMS_DATA_DIRECTION, m_nDmxDataDirection);
	}
#endif
}

void DmxGpioParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((DmxGpioParams *) p)->callbackFunction(s);
}

bool DmxGpioParams::IsMaskSet(uint16_t mask) const {
	return (m_nSetList & mask) == mask;
}
