/**
 * @file l6470params.cpp
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "l6470params.h"
#include "l6470paramsconst.h"
#include "l6470.h"

#include "l6470dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

L6470Params::L6470Params(L6470ParamsStore *pL6470ParamsStore): m_pL6470ParamsStore(pL6470ParamsStore) {
	memset( &m_tL6470Params, 0, sizeof(struct TL6470Params));

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));
}

bool L6470Params::Load(uint8_t nMotorIndex) {
	m_aFileName[5] = nMotorIndex + '0';

	m_tL6470Params.nSetList = 0;

	ReadConfigFile configfile(L6470Params::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pL6470ParamsStore != nullptr) {
			m_pL6470ParamsStore->Update(nMotorIndex, &m_tL6470Params);
		}
	} else if (m_pL6470ParamsStore != nullptr) {
		m_pL6470ParamsStore->Copy(nMotorIndex, &m_tL6470Params);
	} else {
		return false;
	}

	return true;
}

void L6470Params::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pL6470ParamsStore != nullptr);

	if (m_pL6470ParamsStore == nullptr) {
		return;
	}

	m_tL6470Params.nSetList = 0;

	ReadConfigFile config(L6470Params::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pL6470ParamsStore->Update(nMotorIndex, &m_tL6470Params);
}

void L6470Params::Builder(uint8_t nMotorIndex, const struct TL6470Params *ptL6470Params, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	m_aFileName[5] = nMotorIndex + '0';

	if (ptL6470Params != nullptr) {
		memcpy(&m_tL6470Params, ptL6470Params, sizeof(struct TL6470Params));
	} else {
		m_pL6470ParamsStore->Copy(nMotorIndex, &m_tL6470Params);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(L6470ParamsConst::MIN_SPEED, m_tL6470Params.fMinSpeed, isMaskSet(L6470ParamsMask::MIN_SPEED));
	builder.Add(L6470ParamsConst::MAX_SPEED, m_tL6470Params.fMaxSpeed, isMaskSet(L6470ParamsMask::MAX_SPEED));
	builder.Add(L6470ParamsConst::ACC, m_tL6470Params.fAcc, isMaskSet(L6470ParamsMask::ACC));
	builder.Add(L6470ParamsConst::DEC, m_tL6470Params.fDec, isMaskSet(L6470ParamsMask::DEC));
	builder.Add(L6470ParamsConst::KVAL_HOLD, m_tL6470Params.nKvalHold, isMaskSet(L6470ParamsMask::KVAL_HOLD));
	builder.Add(L6470ParamsConst::KVAL_RUN, m_tL6470Params.nKvalRun, isMaskSet(L6470ParamsMask::KVAL_RUN));
	builder.Add(L6470ParamsConst::KVAL_ACC, m_tL6470Params.nKvalAcc, isMaskSet(L6470ParamsMask::KVAL_ACC));
	builder.Add(L6470ParamsConst::KVAL_DEC, m_tL6470Params.nKvalDec, isMaskSet(L6470ParamsMask::KVAL_DEC));
	builder.Add(L6470ParamsConst::MICRO_STEPS, m_tL6470Params.nMicroSteps, isMaskSet(L6470ParamsMask::MICRO_STEPS));

	nSize = builder.GetSize();

	return;
}

void L6470Params::Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pL6470ParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);

	return;
}

void L6470Params::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	float fValue;

	if (Sscan::Float(pLine, L6470ParamsConst::MIN_SPEED, fValue) == Sscan::OK) {
		m_tL6470Params.fMinSpeed = fValue;
		m_tL6470Params.nSetList |= L6470ParamsMask::MIN_SPEED;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::MAX_SPEED, fValue) == Sscan::OK) {
		m_tL6470Params.fMaxSpeed = fValue;
		m_tL6470Params.nSetList |= L6470ParamsMask::MAX_SPEED;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::ACC, fValue) == Sscan::OK) {
		m_tL6470Params.fAcc = fValue;
		m_tL6470Params.nSetList |= L6470ParamsMask::ACC;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::DEC, fValue) == Sscan::OK) {
		m_tL6470Params.fDec = fValue;
		m_tL6470Params.nSetList |= L6470ParamsMask::DEC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_HOLD, nValue8) == Sscan::OK) {
		m_tL6470Params.nKvalHold = nValue8;
		m_tL6470Params.nSetList |= L6470ParamsMask::KVAL_HOLD;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_RUN, nValue8) == Sscan::OK) {
		m_tL6470Params.nKvalRun = nValue8;
		m_tL6470Params.nSetList |= L6470ParamsMask::KVAL_RUN;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_ACC, nValue8) == Sscan::OK) {
		m_tL6470Params.nKvalAcc = nValue8;
		m_tL6470Params.nSetList |= L6470ParamsMask::KVAL_ACC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_DEC, nValue8) == Sscan::OK) {
		m_tL6470Params.nKvalDec = nValue8;
		m_tL6470Params.nSetList |= L6470ParamsMask::KVAL_DEC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::MICRO_STEPS, nValue8) == Sscan::OK) {
		m_tL6470Params.nMicroSteps = nValue8;
		m_tL6470Params.nSetList |= L6470ParamsMask::MICRO_STEPS;
		return;
	}
}

void L6470Params::Set(L6470 *pL6470) {
	assert(pL6470 != nullptr);

	if (isMaskSet(L6470ParamsMask::MIN_SPEED)) {
		pL6470->setMinSpeed(m_tL6470Params.fMinSpeed);
	}

	if (isMaskSet(L6470ParamsMask::MAX_SPEED)) {
		pL6470->setMaxSpeed(m_tL6470Params.fMaxSpeed);
	}

	if (isMaskSet(L6470ParamsMask::ACC)) {
		pL6470->setAcc(m_tL6470Params.fAcc);
	}

	if (isMaskSet(L6470ParamsMask::DEC)) {
		pL6470->setDec(m_tL6470Params.fDec);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_HOLD)) {
		pL6470->setHoldKVAL(m_tL6470Params.nKvalHold);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_RUN)) {
		pL6470->setRunKVAL(m_tL6470Params.nKvalRun);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_ACC)) {
		pL6470->setAccKVAL(m_tL6470Params.nKvalAcc);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_DEC)) {
		pL6470->setDecKVAL(m_tL6470Params.nKvalDec);
	}

	if (isMaskSet(L6470ParamsMask::MICRO_STEPS)) {
		pL6470->setMicroSteps(m_tL6470Params.nMicroSteps);
	}
}

void L6470Params::Dump() {
#ifndef NDEBUG
	if (m_tL6470Params.nSetList == 0) {
		return;
	}

	if (isMaskSet(L6470ParamsMask::MIN_SPEED)) {
		printf(" %s=%f\n", L6470ParamsConst::MIN_SPEED, m_tL6470Params.fMinSpeed);
	}

	if (isMaskSet(L6470ParamsMask::MAX_SPEED)) {
		printf(" %s=%f\n", L6470ParamsConst::MAX_SPEED, m_tL6470Params.fMaxSpeed);
	}

	if (isMaskSet(L6470ParamsMask::ACC)) {
		printf(" %s=%f\n", L6470ParamsConst::ACC, m_tL6470Params.fAcc);
	}

	if (isMaskSet(L6470ParamsMask::DEC)) {
		printf(" %s=%f\n", L6470ParamsConst::DEC, m_tL6470Params.fDec);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_HOLD)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_HOLD, m_tL6470Params.nKvalHold);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_RUN)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_RUN, m_tL6470Params.nKvalRun);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_ACC)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_ACC, m_tL6470Params.nKvalAcc);
	}

	if (isMaskSet(L6470ParamsMask::KVAL_DEC)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_DEC, m_tL6470Params.nKvalDec);
	}

	if (isMaskSet(L6470ParamsMask::MICRO_STEPS)) {
		printf(" %s=%d\n", L6470ParamsConst::MICRO_STEPS, static_cast<int>(m_tL6470Params.nMicroSteps));
	}
#endif
}

void L6470Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<L6470Params*>(p))->callbackFunction(s);
}
