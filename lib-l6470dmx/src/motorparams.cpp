/**
 * @file motorparams.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)
# if __GNUC__ < 9
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wuseless-cast"	//FIXME GCC 8.0.3 Raspbian GNU/Linux 10 (buster)
# endif
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "motorparams.h"
#include "motorparamsconst.h"

#include "l6470dmxconst.h"
#include "l6470dmxstore.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define TICK_S	0.00000025f	///< 250ns

MotorParams::MotorParams() {
	memset(&m_Params, 0, sizeof(struct motorparams::Params));

	m_Params.fStepAngel = 1.8f;	// 200 steps per revolution.

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));
}

bool MotorParams::Load(uint32_t nMotorIndex) {
	DEBUG_ENTRY

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	m_Params.nSetList = 0;

	ReadConfigFile configfile(MotorParams::StaticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		MotorParamsStore::Update(nMotorIndex, &m_Params);
	} else {
		MotorParamsStore::Copy(nMotorIndex, &m_Params);
	}

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
	return true;
}

void MotorParams::Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(MotorParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	MotorParamsStore::Update(nMotorIndex, &m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void MotorParams::Builder(uint32_t nMotorIndex, const struct motorparams::Params *ptMotorParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	if (ptMotorParams != nullptr) {
		memcpy(&m_Params, ptMotorParams, sizeof(struct motorparams::Params));
	} else {
		MotorParamsStore::Copy(nMotorIndex, &m_Params);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(MotorParamsConst::STEP_ANGEL, m_Params.fStepAngel, isMaskSet(motorparams::Mask::STEP_ANGEL));
	builder.Add(MotorParamsConst::VOLTAGE, m_Params.fVoltage, isMaskSet(motorparams::Mask::VOLTAGE));
	builder.Add(MotorParamsConst::CURRENT, m_Params.fCurrent, isMaskSet(motorparams::Mask::CURRENT));
	builder.Add(MotorParamsConst::RESISTANCE, m_Params.fResistance, isMaskSet(motorparams::Mask::RESISTANCE));
	builder.Add(MotorParamsConst::INDUCTANCE, m_Params.fInductance, isMaskSet(motorparams::Mask::INDUCTANCE));

	nSize = builder.GetSize();

	return;
}

void MotorParams::Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);
	DEBUG_EXIT
	return;
}

void MotorParams::callbackFunction(const char *pLine) {
	float f;

	if (Sscan::Float(pLine, MotorParamsConst::STEP_ANGEL, f) == Sscan::OK) {
		if (f != 0) {
			m_Params.fStepAngel = f;
			m_Params.nSetList |= motorparams::Mask::STEP_ANGEL;
		}
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::VOLTAGE, f) == Sscan::OK) {
		m_Params.fVoltage = f;
		m_Params.nSetList |= motorparams::Mask::VOLTAGE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::CURRENT, f) == Sscan::OK) {
		m_Params.fCurrent = f;
		m_Params.nSetList |= motorparams::Mask::CURRENT;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::RESISTANCE, f) == Sscan::OK) {
		m_Params.fResistance = f;
		m_Params.nSetList |= motorparams::Mask::RESISTANCE;
		return;
	}

	if (Sscan::Float(pLine, MotorParamsConst::INDUCTANCE, f) == Sscan::OK) {
		m_Params.fInductance = f;
		m_Params.nSetList |= motorparams::Mask::INDUCTANCE;
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
	if(isMaskSet(motorparams::Mask::STEP_ANGEL)) {
		printf(" %s=%.1f degree\n", MotorParamsConst::STEP_ANGEL, m_Params.fStepAngel);
	}

	if(isMaskSet(motorparams::Mask::VOLTAGE)) {
		printf(" %s=%.2f V\n", MotorParamsConst::VOLTAGE, m_Params.fVoltage);
	}

	if(isMaskSet(motorparams::Mask::CURRENT)) {
		printf(" %s=%.1f A/phase\n", MotorParamsConst::CURRENT, m_Params.fCurrent);
	}

	if(isMaskSet(motorparams::Mask::RESISTANCE)) {
		printf(" %s=%.1f Ohm/phase\n", MotorParamsConst::RESISTANCE, m_Params.fResistance);
	}

	if(isMaskSet(motorparams::Mask::INDUCTANCE)) {
		printf(" %s=%.1f mH/phase\n", MotorParamsConst::INDUCTANCE, m_Params.fInductance);
	}

	float f;

	if ((f = calcIntersectSpeed()) != 0) {
		printf(" Intersect speed = %f step/s (register:INT_SPEED=0x%.4X)\n", f, static_cast<unsigned int>(calcIntersectSpeedReg(f)));
	}
}

float MotorParams::calcIntersectSpeed() {
	if (isMaskSet(motorparams::Mask::RESISTANCE) && isMaskSet(motorparams::Mask::INDUCTANCE)) {
		return (4.0f * m_Params.fResistance) / static_cast<float>(2.0 * M_PI * m_Params.fInductance * 0.001);
	}

	return 0;
}

uint32_t MotorParams::calcIntersectSpeedReg(float f) const {
	return static_cast<uint32_t>(f * (TICK_S * (1U << 26)));
}

void MotorParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<MotorParams*>(p))->callbackFunction(s);
}
