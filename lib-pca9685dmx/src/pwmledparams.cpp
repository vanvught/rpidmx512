/**
 * @file pwmledparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#if defined(__linux__)
 #include <string.h>
#elif defined(__circle__)
#else
 #include "util.h"
#endif

#include "pwmledparams.h"
#include "pwmled.h"
#include "pwmleddmx.h"

#include "pca9685.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_PWM_FREQUENCY_MASK	1<<0
#define SET_OUTPUT_INVERT_MASK	1<<1
#define SET_OUTPUT_DRIVER_MASK	1<<2

static const char PARAMS_FILE_NAME[] ALIGNED = "pwmled.txt";
static const char PARAMS_PWM_FREQUENCY[] ALIGNED = "pwm_frequency";
static const char PARAMS_OUTPUT_INVERT[] ALIGNED = "output_invert";
static const char PARAMS_OUTPUT_DRIVER[] ALIGNED = "output_driver";

PWMLedParams::PWMLedParams(void) :
		Params(PARAMS_FILE_NAME),
		m_bSetList(0),
		m_nPwmFrequency(PWMLED_DEFAULT_FREQUENCY),
		m_bOutputInvert(false), // Output logic state not inverted. Value to use when external driver used.
		m_bOutputDriver(true)	// The 16 LEDn outputs are configured with a totem pole structure.
{
	ReadConfigFile configfile(PWMLedParams::staticCallbackFunction, this);
	configfile.Read(PARAMS_FILE_NAME);
}

PWMLedParams::~PWMLedParams(void) {
}

void PWMLedParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((PWMLedParams *) p)->callbackFunction(s);
}

void PWMLedParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;

	if (Sscan::Uint16(pLine, PARAMS_PWM_FREQUENCY, &value16) == SSCAN_OK) {
		if ((value16 >= PCA9685_FREQUENCY_MIN) && (value16 <= PCA9685_FREQUENCY_MAX)) {
			m_nPwmFrequency = value16;
			m_bSetList |= SET_PWM_FREQUENCY_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_OUTPUT_INVERT, &value8) == SSCAN_OK) {
		if (value8 != 0) {
			m_bOutputInvert = true;
			m_bSetList |= SET_OUTPUT_INVERT_MASK;
			return;
		}
	}

	if (Sscan::Uint8(pLine, PARAMS_OUTPUT_DRIVER, &value8) == SSCAN_OK) {
		if (value8 == 0) {
			m_bOutputDriver = false;
			m_bSetList |= SET_OUTPUT_DRIVER_MASK;
			return;
		}
	}

}

void PWMLedParams::Set(PWMLedDmx *pPWMLedDMX) {
	assert(pPWMLedDMX != 0);

	bool isSet;

	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	const uint8_t i2cAddress = GetI2cAddress(&isSet);
	if (isSet) {
		pPWMLedDMX->SetI2cAddress(i2cAddress);
	}

	const uint16_t DmxStartAddress = GetDmxStartAddress(&isSet);
	if (isSet) {
		pPWMLedDMX->SetDmxStartAddress(DmxStartAddress);
	}

	const uint8_t BoardInstances = GetBoardInstances(&isSet);
	if (isSet) {
		pPWMLedDMX->SetBoardInstances(BoardInstances);
	}

	const char *p = GetDmxSlotInfoRaw(&isSet);
	if (isSet) {
		pPWMLedDMX->SetSlotInfoRaw(p);
	}

	// Footprint overwrites board instances!
	const uint16_t DmxFootprint = GetDmxFootprint(&isSet);
	if (isSet) {
		pPWMLedDMX->SetDmxFootprint(DmxFootprint);
	}

	if(IsMaskSet(SET_PWM_FREQUENCY_MASK)) {
		pPWMLedDMX->SetPwmfrequency(m_nPwmFrequency);
	}

	if(IsMaskSet(SET_OUTPUT_INVERT_MASK)) {
		pPWMLedDMX->SetInvert(m_bOutputInvert);
	}

	if(IsMaskSet(SET_OUTPUT_DRIVER_MASK)) {
		pPWMLedDMX->SetOutDriver(m_bOutputDriver);
	}
}

void PWMLedParams::Dump(void) {
#ifndef NDEBUG
	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	printf("PWM led params \'%s\':\n", PARAMS_FILE_NAME);

	if(IsMaskSet(SET_PWM_FREQUENCY_MASK)) {
		printf("%s=%d Hz\n", PARAMS_PWM_FREQUENCY, m_nPwmFrequency);
	}

	if(IsMaskSet(SET_OUTPUT_INVERT_MASK)) {
		printf("%s=%d [Output logic state %sinverted]\n", PARAMS_OUTPUT_INVERT, (int) m_bOutputInvert, m_bOutputInvert ? "" : "not ");
	}

	if(IsMaskSet(SET_OUTPUT_DRIVER_MASK)) {
		printf("%s=%d [The 16 LEDn outputs are configured with %s structure]\n", PARAMS_OUTPUT_DRIVER, (int) m_bOutputDriver, m_bOutputDriver ? "a totem pole" : "an open-drain");
	}

	Params::Dump();
#endif
}

bool PWMLedParams::IsMaskSet(uint16_t mask) const {
	return (m_bSetList & mask) == mask;
}
