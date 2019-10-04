/**
 * @file motorparams.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#pragma GCC push_options
#pragma GCC optimize ("Os")

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "motorparams.h"
#include "motorparamsconst.h"

#include "l6470dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define TICK_S	0.00000025	///< 250ns

MotorParams::MotorParams(MotorParamsStore *pMotorParamsStore): m_pMotorParamsStore(pMotorParamsStore) {
	uint8_t *p = (uint8_t *) &m_tMotorParams;

	for (uint32_t i = 0; i < sizeof(struct TMotorParams); i++) {
		*p++ = 0;
	}

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	const char *src = (char *)L6470DmxConst::FILE_NAME_MOTOR;
	strncpy(m_aFileName, src, sizeof(m_aFileName));
}

MotorParams::~MotorParams(void) {
	m_tMotorParams.nSetList = 0;
}

bool MotorParams::Load(uint8_t nMotorIndex) {
	m_aFileName[5] = (char) nMotorIndex + '0';

	m_tMotorParams.nSetList = 0;

	ReadConfigFile configfile(MotorParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pMotorParamsStore != 0) {
			m_pMotorParamsStore->Update(nMotorIndex, &m_tMotorParams);
		}
	} else if (m_pMotorParamsStore != 0) {
		m_pMotorParamsStore->Copy(nMotorIndex, &m_tMotorParams);
	} else {
		return false;
	}

	return true;
}

void MotorParams::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pMotorParamsStore != 0);

	if (m_pMotorParamsStore == 0) {
		return;
	}

	m_tMotorParams.nSetList = 0;

	ReadConfigFile config(MotorParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pMotorParamsStore->Update(nMotorIndex, &m_tMotorParams);
}

bool MotorParams::Builder(uint8_t nMotorIndex, const struct TMotorParams *ptMotorParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	m_aFileName[5] = (char) nMotorIndex + '0';

	if (ptMotorParams != 0) {
		memcpy(&m_tMotorParams, ptMotorParams, sizeof(struct TMotorParams));
	} else {
		m_pMotorParamsStore->Copy(nMotorIndex, &m_tMotorParams);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	bool isAdded = builder.Add(MotorParamsConst::STEP_ANGEL, m_tMotorParams.fStepAngel, isMaskSet(MOTOR_PARAMS_MASK_STEP_ANGEL));
	isAdded &= builder.Add(MotorParamsConst::VOLTAGE, m_tMotorParams.fVoltage, isMaskSet(MOTOR_PARAMS_MASK_VOLTAGE));
	isAdded &= builder.Add(MotorParamsConst::CURRENT, m_tMotorParams.fCurrent, isMaskSet(MOTOR_PARAMS_MASK_CURRENT));
	isAdded &= builder.Add(MotorParamsConst::RESISTANCE, m_tMotorParams.fResistance, isMaskSet(MOTOR_PARAMS_MASK_RESISTANCE));
	isAdded &= builder.Add(MotorParamsConst::INDUCTANCE, m_tMotorParams.fInductance, isMaskSet(MOTOR_PARAMS_MASK_INDUCTANCE));

	nSize = builder.GetSize();

	return isAdded;
}

bool MotorParams::Save(uint8_t nMotorIndex, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pMotorParamsStore == 0) {
		nSize = 0;
		return false;
	}

	return Builder(nMotorIndex, 0, pBuffer, nLength, nSize);
}

void MotorParams::callbackFunction(const char *pLine) {
	float f;

	if (Sscan::Float(pLine, MotorParamsConst::STEP_ANGEL, &f) == SSCAN_OK) {
		m_tMotorParams.fStepAngel = f;
		m_tMotorParams.nSetList |= MOTOR_PARAMS_MASK_STEP_ANGEL;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::VOLTAGE, &f) == SSCAN_OK) {
		m_tMotorParams.fVoltage = f;
		m_tMotorParams.nSetList |= MOTOR_PARAMS_MASK_VOLTAGE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::CURRENT, &f) == SSCAN_OK) {
		m_tMotorParams.fCurrent = f;
		m_tMotorParams.nSetList |= MOTOR_PARAMS_MASK_CURRENT;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::RESISTANCE, &f) == SSCAN_OK) {
		m_tMotorParams.fResistance = f;
		m_tMotorParams.nSetList |= MOTOR_PARAMS_MASK_RESISTANCE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::INDUCTANCE, &f) == SSCAN_OK) {
		m_tMotorParams.fInductance = f;
		m_tMotorParams.nSetList |= MOTOR_PARAMS_MASK_INDUCTANCE;
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

	if (m_tMotorParams.nSetList == 0) {
		return;
	}

	if(isMaskSet(MOTOR_PARAMS_MASK_STEP_ANGEL)) {
		printf(" %s=%.1f degree\n", MotorParamsConst::STEP_ANGEL, m_tMotorParams.fStepAngel);
	}

	if(isMaskSet(MOTOR_PARAMS_MASK_VOLTAGE)) {
		printf(" %s=%.2f V\n", MotorParamsConst::VOLTAGE, m_tMotorParams.fVoltage);
	}

	if(isMaskSet(MOTOR_PARAMS_MASK_CURRENT)) {
		printf(" %s=%.1f A/phase\n", MotorParamsConst::CURRENT, m_tMotorParams.fCurrent);
	}

	if(isMaskSet(MOTOR_PARAMS_MASK_RESISTANCE)) {
		printf(" %s=%.1f Ohm/phase\n", MotorParamsConst::RESISTANCE, m_tMotorParams.fResistance);
	}

	if(isMaskSet(MOTOR_PARAMS_MASK_INDUCTANCE)) {
		printf(" %s=%.1f mH/phase\n", MotorParamsConst::INDUCTANCE, m_tMotorParams.fInductance);
	}

	if ((f = calcIntersectSpeed()) != (float) 0) {
		printf(" Intersect speed = %f step/s (register:INT_SPEED=0x%.4X)\n", f, (unsigned int) calcIntersectSpeedReg(f));
	}
#endif
}

float MotorParams::calcIntersectSpeed(void) const {
	if (isMaskSet(MOTOR_PARAMS_MASK_RESISTANCE) && isMaskSet(MOTOR_PARAMS_MASK_INDUCTANCE)) {
		return ((float) 4 * m_tMotorParams.fResistance) / (2 * M_PI * m_tMotorParams.fInductance * 0.001);
	} else {
		return (float) 0;
	}
}

uint32_t MotorParams::calcIntersectSpeedReg(float f) const {
	return (f * (TICK_S * (1 << 26)));
}

void MotorParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((MotorParams *) p)->callbackFunction(s);
}

bool MotorParams::isMaskSet(uint32_t nMask) const {
	return (m_tMotorParams.nSetList & nMask) == nMask;
}
