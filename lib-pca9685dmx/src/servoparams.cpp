/**
 * @file servoparams.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#if defined(__linux__)
 #define ALIGNED
 #include <string.h>
#elif defined(__circle__)
 #define ALIGNED
 #include "circle/util.h"
#else
 #include "util.h"
#endif

#include "params.h"

#include "servoparams.h"
#include "servo.h"
#include "servodmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#define LEFT_US_MASK	1<<0
#define RIGHT_US_MASK	1<<1

static const char PARAMS_FILE_NAME[] ALIGNED = "servo.txt";
static const char PARAMS_LEFT_US[] ALIGNED = "left_us";
static const char PARAMS_RIGHT_US[] ALIGNED = "right_us";

ServoParams::ServoParams(void): Params(PARAMS_FILE_NAME), m_bSetList(0), m_nLeftUs(SERVO_LEFT_DEFAULT_US), m_nRightUs(SERVO_RIGHT_DEFAULT_US) {

	ReadConfigFile configfile(ServoParams::staticCallbackFunction, this);
	configfile.Read(PARAMS_FILE_NAME);
}

ServoParams::~ServoParams(void) {

}

void ServoParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ServoParams *) p)->callbackFunction(s);
}

void ServoParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint16_t value16;

	if (Sscan::Uint16(pLine, PARAMS_LEFT_US, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 < m_nRightUs)) {
			m_nLeftUs = value16;
			m_bSetList |= LEFT_US_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_RIGHT_US, &value16) == SSCAN_OK) {
		if (value16 > m_nLeftUs) {
			m_nRightUs = value16;
			m_bSetList |= RIGHT_US_MASK;
		}
		return;
	}
}

void ServoParams::Set(ServoDMX *pServoDmx) {
	assert(pServoDmx != 0);

	bool isSet;

	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	const uint8_t i2cAddress = GetI2cAddress(&isSet);
	if (isSet) {
		pServoDmx->SetI2cAddress(i2cAddress);
	}

	const uint16_t DmxStartAddress = GetDmxStartAddress(&isSet);
	if (isSet) {
		pServoDmx->SetDmxStartAddress(DmxStartAddress);
	}

	const uint8_t BoardInstances = GetBoardInstances(&isSet);
	if (isSet) {
		pServoDmx->SetBoardInstances(BoardInstances);
	}

	if(IsMaskSet(LEFT_US_MASK)) {
		pServoDmx->SetLeftUs(m_nLeftUs);
	}

	if(IsMaskSet(RIGHT_US_MASK)) {
		pServoDmx->SetRightUs(m_nRightUs);
	}
}

void ServoParams::Dump(void) {
	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	printf("Servo params \'%s\':\n", PARAMS_FILE_NAME);

	if(IsMaskSet(LEFT_US_MASK)) {
		printf("%s=%d\n", PARAMS_LEFT_US, m_nLeftUs);
	}

	if(IsMaskSet(RIGHT_US_MASK)) {
		printf("%s=%d\n", PARAMS_RIGHT_US, m_nRightUs);
	}

	Params::Dump();
}

bool ServoParams::IsMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}
