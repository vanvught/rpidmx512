/**
 * @file motorparams.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# if __GNUC__ < 9
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wuseless-cast"	//FIXME GCC 8.0.3 Raspbian GNU/Linux 10 (buster)
# endif
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

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
	memset(&m_tMotorParams, 0, sizeof(struct TMotorParams));

	m_tMotorParams.fStepAngel = 1.8;	// 200 steps per revolution.

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));
}

bool MotorParams::Load(uint8_t nMotorIndex) {
	m_aFileName[5] = nMotorIndex + '0';

	m_tMotorParams.nSetList = 0;

	ReadConfigFile configfile(MotorParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pMotorParamsStore != nullptr) {
			m_pMotorParamsStore->Update(nMotorIndex, &m_tMotorParams);
		}
	} else if (m_pMotorParamsStore != nullptr) {
		m_pMotorParamsStore->Copy(nMotorIndex, &m_tMotorParams);
	} else {
		return false;
	}

	return true;
}

void MotorParams::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pMotorParamsStore != nullptr);

	if (m_pMotorParamsStore == nullptr) {
		return;
	}

	m_tMotorParams.nSetList = 0;

	ReadConfigFile config(MotorParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pMotorParamsStore->Update(nMotorIndex, &m_tMotorParams);
}

void MotorParams::Builder(uint8_t nMotorIndex, const struct TMotorParams *ptMotorParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	m_aFileName[5] = nMotorIndex + '0';

	if (ptMotorParams != nullptr) {
		memcpy(&m_tMotorParams, ptMotorParams, sizeof(struct TMotorParams));
	} else {
		m_pMotorParamsStore->Copy(nMotorIndex, &m_tMotorParams);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(MotorParamsConst::STEP_ANGEL, m_tMotorParams.fStepAngel, isMaskSet(MotorParamsMask::STEP_ANGEL));
	builder.Add(MotorParamsConst::VOLTAGE, m_tMotorParams.fVoltage, isMaskSet(MotorParamsMask::VOLTAGE));
	builder.Add(MotorParamsConst::CURRENT, m_tMotorParams.fCurrent, isMaskSet(MotorParamsMask::CURRENT));
	builder.Add(MotorParamsConst::RESISTANCE, m_tMotorParams.fResistance, isMaskSet(MotorParamsMask::RESISTANCE));
	builder.Add(MotorParamsConst::INDUCTANCE, m_tMotorParams.fInductance, isMaskSet(MotorParamsMask::INDUCTANCE));

	nSize = builder.GetSize();

	return;
}

void MotorParams::Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pMotorParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);

	return;
}

void MotorParams::callbackFunction(const char *pLine) {
	float f;

	if (Sscan::Float(pLine, MotorParamsConst::STEP_ANGEL, f) == Sscan::OK) {
		if (f != 0) {
			m_tMotorParams.fStepAngel = f;
			m_tMotorParams.nSetList |= MotorParamsMask::STEP_ANGEL;
		}
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::VOLTAGE, f) == Sscan::OK) {
		m_tMotorParams.fVoltage = f;
		m_tMotorParams.nSetList |= MotorParamsMask::VOLTAGE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::CURRENT, f) == Sscan::OK) {
		m_tMotorParams.fCurrent = f;
		m_tMotorParams.nSetList |= MotorParamsMask::CURRENT;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::RESISTANCE, f) == Sscan::OK) {
		m_tMotorParams.fResistance = f;
		m_tMotorParams.nSetList |= MotorParamsMask::RESISTANCE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::INDUCTANCE, f) == Sscan::OK) {
		m_tMotorParams.fInductance = f;
		m_tMotorParams.nSetList |= MotorParamsMask::INDUCTANCE;
		return;
	}
}

void MotorParams::Set(L6470 *pL6470) {
	assert(pL6470 != nullptr);

	float f;

	if ((f = calcIntersectSpeed()) != 0) {
		pL6470->setParam(L6470_PARAM_INT_SPD, calcIntersectSpeedReg(f));
	}
}

void MotorParams::Dump() {
#ifndef NDEBUG
	float f;

	if (m_tMotorParams.nSetList == 0) {
		return;
	}

	if(isMaskSet(MotorParamsMask::STEP_ANGEL)) {
		printf(" %s=%.1f degree\n", MotorParamsConst::STEP_ANGEL, m_tMotorParams.fStepAngel);
	}

	if(isMaskSet(MotorParamsMask::VOLTAGE)) {
		printf(" %s=%.2f V\n", MotorParamsConst::VOLTAGE, m_tMotorParams.fVoltage);
	}

	if(isMaskSet(MotorParamsMask::CURRENT)) {
		printf(" %s=%.1f A/phase\n", MotorParamsConst::CURRENT, m_tMotorParams.fCurrent);
	}

	if(isMaskSet(MotorParamsMask::RESISTANCE)) {
		printf(" %s=%.1f Ohm/phase\n", MotorParamsConst::RESISTANCE, m_tMotorParams.fResistance);
	}

	if(isMaskSet(MotorParamsMask::INDUCTANCE)) {
		printf(" %s=%.1f mH/phase\n", MotorParamsConst::INDUCTANCE, m_tMotorParams.fInductance);
	}

	if ((f = calcIntersectSpeed()) != 0) {
		printf(" Intersect speed = %f step/s (register:INT_SPEED=0x%.4X)\n", f, static_cast<unsigned int>(calcIntersectSpeedReg(f)));
	}
#endif
}

float MotorParams::calcIntersectSpeed() {
	if (isMaskSet(MotorParamsMask::RESISTANCE) && isMaskSet(MotorParamsMask::INDUCTANCE)) {
		return (4 * m_tMotorParams.fResistance) / (2 * M_PI * m_tMotorParams.fInductance * 0.001);
	}

	return 0;
}

uint32_t MotorParams::calcIntersectSpeedReg(float f) const {
	return (f * (TICK_S * (1 << 26)));
}

void MotorParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<MotorParams*>(p))->callbackFunction(s);
}

