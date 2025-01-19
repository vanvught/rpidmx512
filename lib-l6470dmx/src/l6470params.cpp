/**
 * @file l6470params.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "l6470params.h"
#include "l6470paramsconst.h"
#include "l6470.h"

#include "l6470dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

L6470Params::L6470Params() {
	memset( &m_Params, 0, sizeof(struct l6470params::Params));

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));
}

void L6470Params::Load(uint32_t nMotorIndex) {
	DEBUG_ENTRY

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	m_Params.nSetList = 0;

	ReadConfigFile configfile(L6470Params::StaticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		L6470ParamsStore::Update(nMotorIndex, &m_Params);
	} else {
		L6470ParamsStore::Copy(nMotorIndex, &m_Params);
	}

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void L6470Params::Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(L6470Params::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	L6470ParamsStore::Update(nMotorIndex, &m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void L6470Params::Builder(uint32_t nMotorIndex, const struct l6470params::Params *ptL6470Params, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	if (ptL6470Params != nullptr) {
		memcpy(&m_Params, ptL6470Params, sizeof(struct l6470params::Params));
	} else {
		L6470ParamsStore::Copy(nMotorIndex, &m_Params);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(L6470ParamsConst::MIN_SPEED, m_Params.fMinSpeed, isMaskSet(l6470params::Mask::MIN_SPEED));
	builder.Add(L6470ParamsConst::MAX_SPEED, m_Params.fMaxSpeed, isMaskSet(l6470params::Mask::MAX_SPEED));
	builder.Add(L6470ParamsConst::ACC, m_Params.fAcc, isMaskSet(l6470params::Mask::ACC));
	builder.Add(L6470ParamsConst::DEC, m_Params.fDec, isMaskSet(l6470params::Mask::DEC));
	builder.Add(L6470ParamsConst::KVAL_HOLD, m_Params.nKvalHold, isMaskSet(l6470params::Mask::KVAL_HOLD));
	builder.Add(L6470ParamsConst::KVAL_RUN, m_Params.nKvalRun, isMaskSet(l6470params::Mask::KVAL_RUN));
	builder.Add(L6470ParamsConst::KVAL_ACC, m_Params.nKvalAcc, isMaskSet(l6470params::Mask::KVAL_ACC));
	builder.Add(L6470ParamsConst::KVAL_DEC, m_Params.nKvalDec, isMaskSet(l6470params::Mask::KVAL_DEC));
	builder.Add(L6470ParamsConst::MICRO_STEPS, m_Params.nMicroSteps, isMaskSet(l6470params::Mask::MICRO_STEPS));

	nSize = builder.GetSize();

	return;
}

void L6470Params::Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_EXIT
	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);
	DEBUG_EXIT
}

void L6470Params::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	float fValue;

	if (Sscan::Float(pLine, L6470ParamsConst::MIN_SPEED, fValue) == Sscan::OK) {
		m_Params.fMinSpeed = fValue;
		m_Params.nSetList |= l6470params::Mask::MIN_SPEED;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::MAX_SPEED, fValue) == Sscan::OK) {
		m_Params.fMaxSpeed = fValue;
		m_Params.nSetList |= l6470params::Mask::MAX_SPEED;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::ACC, fValue) == Sscan::OK) {
		m_Params.fAcc = fValue;
		m_Params.nSetList |= l6470params::Mask::ACC;
		return;
	}

	if (Sscan::Float(pLine, L6470ParamsConst::DEC, fValue) == Sscan::OK) {
		m_Params.fDec = fValue;
		m_Params.nSetList |= l6470params::Mask::DEC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_HOLD, nValue8) == Sscan::OK) {
		m_Params.nKvalHold = nValue8;
		m_Params.nSetList |= l6470params::Mask::KVAL_HOLD;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_RUN, nValue8) == Sscan::OK) {
		m_Params.nKvalRun = nValue8;
		m_Params.nSetList |= l6470params::Mask::KVAL_RUN;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_ACC, nValue8) == Sscan::OK) {
		m_Params.nKvalAcc = nValue8;
		m_Params.nSetList |= l6470params::Mask::KVAL_ACC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::KVAL_DEC, nValue8) == Sscan::OK) {
		m_Params.nKvalDec = nValue8;
		m_Params.nSetList |= l6470params::Mask::KVAL_DEC;
		return;
	}

	if (Sscan::Uint8(pLine, L6470ParamsConst::MICRO_STEPS, nValue8) == Sscan::OK) {
		m_Params.nMicroSteps = nValue8;
		m_Params.nSetList |= l6470params::Mask::MICRO_STEPS;
		return;
	}
}

void L6470Params::Set(L6470 *pL6470) {
	assert(pL6470 != nullptr);

	if (isMaskSet(l6470params::Mask::MIN_SPEED)) {
		pL6470->setMinSpeed(m_Params.fMinSpeed);
	}

	if (isMaskSet(l6470params::Mask::MAX_SPEED)) {
		pL6470->setMaxSpeed(m_Params.fMaxSpeed);
	}

	if (isMaskSet(l6470params::Mask::ACC)) {
		pL6470->setAcc(m_Params.fAcc);
	}

	if (isMaskSet(l6470params::Mask::DEC)) {
		pL6470->setDec(m_Params.fDec);
	}

	if (isMaskSet(l6470params::Mask::KVAL_HOLD)) {
		pL6470->setHoldKVAL(m_Params.nKvalHold);
	}

	if (isMaskSet(l6470params::Mask::KVAL_RUN)) {
		pL6470->setRunKVAL(m_Params.nKvalRun);
	}

	if (isMaskSet(l6470params::Mask::KVAL_ACC)) {
		pL6470->setAccKVAL(m_Params.nKvalAcc);
	}

	if (isMaskSet(l6470params::Mask::KVAL_DEC)) {
		pL6470->setDecKVAL(m_Params.nKvalDec);
	}

	if (isMaskSet(l6470params::Mask::MICRO_STEPS)) {
		pL6470->setMicroSteps(m_Params.nMicroSteps);
	}
}

void L6470Params::Dump() {
	if (isMaskSet(l6470params::Mask::MIN_SPEED)) {
		printf(" %s=%f\n", L6470ParamsConst::MIN_SPEED, m_Params.fMinSpeed);
	}

	if (isMaskSet(l6470params::Mask::MAX_SPEED)) {
		printf(" %s=%f\n", L6470ParamsConst::MAX_SPEED, m_Params.fMaxSpeed);
	}

	if (isMaskSet(l6470params::Mask::ACC)) {
		printf(" %s=%f\n", L6470ParamsConst::ACC, m_Params.fAcc);
	}

	if (isMaskSet(l6470params::Mask::DEC)) {
		printf(" %s=%f\n", L6470ParamsConst::DEC, m_Params.fDec);
	}

	if (isMaskSet(l6470params::Mask::KVAL_HOLD)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_HOLD, m_Params.nKvalHold);
	}

	if (isMaskSet(l6470params::Mask::KVAL_RUN)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_RUN, m_Params.nKvalRun);
	}

	if (isMaskSet(l6470params::Mask::KVAL_ACC)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_ACC, m_Params.nKvalAcc);
	}

	if (isMaskSet(l6470params::Mask::KVAL_DEC)) {
		printf(" %s=%d\n", L6470ParamsConst::KVAL_DEC, m_Params.nKvalDec);
	}

	if (isMaskSet(l6470params::Mask::MICRO_STEPS)) {
		printf(" %s=%d\n", L6470ParamsConst::MICRO_STEPS, static_cast<int>(m_Params.nMicroSteps));
	}
}

void L6470Params::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<L6470Params*>(p))->callbackFunction(s);
}
