/**
 * @file motorparams.cpp
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

#if defined(__linux__)
 #include <string.h>
#elif defined(__circle__)
#else
 #include "util.h"
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "motorparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define TICK_S	0.00000025	///< 250ns

#define SET_STEP_ANGEL_MASK	1<<0
#define SET_VOLTAGE_MASK	1<<1
#define SET_CURRENT_MASK	1<<2
#define SET_RESISTANCE_MASK	1<<3
#define SET_INDUCTANCE_MASK	1<<4

static const char MOTOR_PARAMS_STEP_ANGEL[] ALIGNED = "motor_step_angel";
static const char MOTOR_PARAMS_VOLTAGE[] ALIGNED = "motor_voltage";
static const char MOTOR_PARAMS_CURRENT[] ALIGNED = "motor_current";
static const char MOTOR_PARAMS_RESISTANCE[] ALIGNED = "motor_resistance";
static const char MOTOR_PARAMS_INDUCTANCE[] ALIGNED = "motor_inductance";

MotorParams::MotorParams(const char *pFileName): m_bSetList(0) {
	assert(pFileName != 0);

	m_fStepAngel = 0;
	m_fVoltage = 0;
	m_fCurrent = 0;
	m_fResistance = 0;
	m_fInductance = 0;

	ReadConfigFile configfile(MotorParams::staticCallbackFunction, this);
	configfile.Read(pFileName);
}

MotorParams::~MotorParams(void) {
}

float MotorParams::GetStepAngel(void) {
	return m_fStepAngel;
}

float MotorParams::GetVoltage(void) {
	return m_fVoltage;
}

float MotorParams::GetCurrent(void) {
	return m_fCurrent;
}

float MotorParams::GetResistance(void) {
	return m_fResistance;
}

float MotorParams::GetInductance(void) {
	return m_fInductance;
}

void MotorParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((MotorParams *) p)->callbackFunction(s);
}

void MotorParams::callbackFunction(const char *pLine) {
	float f;

	if (Sscan::Float(pLine, MOTOR_PARAMS_STEP_ANGEL, &f) == SSCAN_OK) {
		m_fStepAngel = f;
		m_bSetList |= SET_STEP_ANGEL_MASK;
		return;
	}

	if (Sscan::Float(pLine, MOTOR_PARAMS_VOLTAGE, &f) == SSCAN_OK) {
		m_fVoltage = f;
		m_bSetList |= SET_VOLTAGE_MASK;
		return;
	}

	if (Sscan::Float(pLine, MOTOR_PARAMS_CURRENT, &f) == SSCAN_OK) {
		m_fCurrent = f;
		m_bSetList |= SET_CURRENT_MASK;
		return;
	}

	if (Sscan::Float(pLine, MOTOR_PARAMS_RESISTANCE, &f) == SSCAN_OK) {
		m_fResistance = f;
		m_bSetList |= SET_RESISTANCE_MASK;
		return;
	}

	if (Sscan::Float(pLine, MOTOR_PARAMS_INDUCTANCE, &f) == SSCAN_OK) {
		m_fInductance = f;
		m_bSetList |= SET_INDUCTANCE_MASK;
		return;
	}
}

void MotorParams::Set(L6470 *pL6470) {
	assert(pL6470 != 0);

	float f;

	if ((f = calcIntersectSpeed()) != (float) 0) {
		pL6470->setParam(L6470_PARAM_INT_SPD, calcIntersectSpeedReg(f));
	}
}

void MotorParams::Dump(void) {
#ifndef NDEBUG
	float f;

	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(SET_STEP_ANGEL_MASK)) {
		printf("%s=%.1f degree\n", MOTOR_PARAMS_STEP_ANGEL, m_fStepAngel);
	}

	if(isMaskSet(SET_VOLTAGE_MASK)) {
		printf("%s=%.2f V\n", MOTOR_PARAMS_VOLTAGE, m_fVoltage);
	}

	if(isMaskSet(SET_CURRENT_MASK)) {
		printf("%s=%.1f A/phase\n", MOTOR_PARAMS_CURRENT, m_fCurrent);
	}

	if(isMaskSet(SET_RESISTANCE_MASK)) {
		printf("%s=%.1f Ohm/phase\n", MOTOR_PARAMS_RESISTANCE, m_fResistance);
	}

	if(isMaskSet(SET_INDUCTANCE_MASK)) {
		printf("%s=%.1f mH/phase\n", MOTOR_PARAMS_INDUCTANCE, m_fInductance);
	}

	if ((f = calcIntersectSpeed()) != (float) 0) {
		printf("Intersect speed = %f step/s (register:INT_SPEED=0x%.4X)\n", f, (unsigned int) calcIntersectSpeedReg(f));
	}
#endif
}

float MotorParams::calcIntersectSpeed(void) const {
	if (isMaskSet(SET_RESISTANCE_MASK) && isMaskSet(SET_INDUCTANCE_MASK)) {
		return ((float) 4 * m_fResistance) / (2 * M_PI * m_fInductance * 0.001);
	} else {
		return (float) 0;
	}
}

uint32_t MotorParams::calcIntersectSpeedReg(float f) const {
	return (f * (TICK_S * (1 << 26)));
}

bool MotorParams::isMaskSet(uint32_t mask) const {
	return (m_bSetList & mask) == mask;
}
