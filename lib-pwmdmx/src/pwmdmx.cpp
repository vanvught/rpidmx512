/**
 * @file pwmdmx.cpp
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

#if defined(__linux__)
  #include <string.h>
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "pwmdmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "pca9685dmxled.h"
#include "pca9685dmxledparams.h"

#include "pca9685dmxservo.h"
#include "pca9685dmxservoparams.h"

#include "tlc59711dmx.h"
#include "tlc59711dmxparams.h"

#define CHIP_MASK	1<<0
#define MODE_MASK	1<<1

static const char PARAMS_FILE_NAME[] ALIGNED = "pwmdmx.txt";
static const char PARAMS_CHIP[] ALIGNED = "chip";
static const char PARAMS_MODE[] ALIGNED = "mode";

const char aChipName[2][9] ALIGNED = { "TLC59711", "PCA5968\0" };
const char aModeName[2][7] ALIGNED = { "PWMLED", "SERVO\0" };

PwmDmx::PwmDmx(void):
	m_bSetList(0),
	m_tPwmDmxMode(PWMDMX_MODE_PWMLED),
	m_tPwmDmxChip(PWMDMX_CHIP_TLC59711),
	m_pLightSet(0)
{
	ReadConfigFile configfile(PwmDmx::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
	}
#ifndef NDEBUG
	else {
		printf("File \"%s\" not found\n", PARAMS_FILE_NAME);
	}
#endif

	if (m_tPwmDmxChip == PWMDMX_CHIP_TLC59711) {
		m_tPwmDmxMode = PWMDMX_MODE_PWMLED;
		TLC59711Dmx *pwmleddmx = new TLC59711Dmx();
		TLC59711DmxParams pwmledparms;
		if (pwmledparms.Load()) {
			pwmledparms.Dump();
			pwmledparms.Set(pwmleddmx);
		}
		pwmleddmx->Start();
		m_pLightSet = pwmleddmx;
	} else if (m_tPwmDmxChip == PWMDMX_CHIP_PCA9685) {
		if (m_tPwmDmxMode == PWMDMX_MODE_SERVO) {
			PCA9685DmxServo *servodmx = new PCA9685DmxServo();
			PCA9685DmxServoParams servoparms;
			if (servoparms.Load()) {
				servoparms.Dump();
				servoparms.Set(servodmx);
			}
			servodmx->Start();
			m_pLightSet = servodmx;
		} else {
			PCA9685DmxLed *pwmleddmx = new PCA9685DmxLed();
			PCA9685DmxLedParams pwmledparms;
			if (pwmledparms.Load()) {
				pwmledparms.Dump();
				pwmledparms.Set(pwmleddmx);
			}
			pwmleddmx->Start();
			m_pLightSet = pwmleddmx;
		}
	}

}

PwmDmx::~PwmDmx(void) {
	if (m_pLightSet != 0) {
		delete m_pLightSet;
		m_pLightSet = 0;
	}
}

TPwmDmxMode PwmDmx::GetPwmDmxMode(bool &IsSet) const {
	IsSet = isMaskSet(MODE_MASK);
	return m_tPwmDmxMode;
}

TPwmDmxChip PwmDmx::GetPwmDmxChip(bool &IsSet) const {
	IsSet = isMaskSet(CHIP_MASK);
	return m_tPwmDmxChip;
}

void PwmDmx::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((PwmDmx *) p)->callbackFunction(s);
}

void PwmDmx::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(CHIP_MASK)) {
		printf(" %s={%d} [%s]\n", PARAMS_CHIP, (int) m_tPwmDmxChip, aChipName[m_tPwmDmxChip]);
	}

	if(isMaskSet(MODE_MASK)) {
		printf(" %s={%d} [%s]\n", PARAMS_MODE, (int) m_tPwmDmxMode, aModeName[m_tPwmDmxMode]);
	}
#endif
}

bool PwmDmx::isMaskSet(uint16_t nMask) const {
	return (m_bSetList & nMask) == nMask;
}

void PwmDmx::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	char name[32];
	uint8_t len = sizeof(name) - 1;

	if (Sscan::Char(pLine, PARAMS_MODE, name, &len) == SSCAN_OK) {
		if (memcmp(name,"pwmled", 6) == 0) {
			m_tPwmDmxMode = PWMDMX_MODE_PWMLED;
			m_bSetList |= MODE_MASK;
		} else if (memcmp(name,"servo", 5) == 0) {
			m_tPwmDmxMode = PWMDMX_MODE_SERVO;
			m_bSetList |= MODE_MASK;
		}
		return;
	}

	len = sizeof(name) - 1;
	if (Sscan::Char(pLine, PARAMS_CHIP, name, &len) == SSCAN_OK) {
		if (memcmp(name,"tlc59711", 8) == 0) {
			m_tPwmDmxChip = PWMDMX_CHIP_TLC59711;
			m_bSetList |= CHIP_MASK;
		} else if (memcmp(name,"pca9685", 7) == 0) {
			m_tPwmDmxChip = PWMDMX_CHIP_PCA9685;
			m_bSetList |= CHIP_MASK;
		}
		return;
	}
}

LightSet* PwmDmx::GetLightSet(void) const {
	return m_pLightSet;
}
