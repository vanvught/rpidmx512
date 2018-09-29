/**
 * @file l6470params.cpp
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

#include "l6470params.h"
#include "l6470.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_MIN_SPEED_MASK		(1 << 0)
#define SET_MAX_SPEED_MASK		(1 << 1)
#define SET_ACC_MASK			(1 << 2)
#define SET_DEC_MASK			(1 << 3)
#define SET_KVAL_HOLD_MASK		(1 << 4)
#define SET_KVAL_RUN_MASK		(1 << 5)
#define SET_KVAL_ACC_MASK		(1 << 6)
#define SET_KVAL_DEC_MASK		(1 << 7)
#define SET_MICRO_STEPS_MASK	(1 << 8)

static const char L6470_PARAMS_MIN_SPEED[] ALIGNED = "l6470_min_speed";
static const char L6470_PARAMS_MAX_SPEED[] ALIGNED = "l6470_max_speed";
static const char L6470_PARAMS_ACC[] ALIGNED = "l6470_acc";
static const char L6470_PARAMS_DEC[] ALIGNED = "l6470_dec";
static const char L6470_PARAMS_KVAL_HOLD[] ALIGNED = "l6470_kval_hold";
static const char L6470_PARAMS_KVAL_RUN[] ALIGNED = "l6470_kval_run";
static const char L6470_PARAMS_KVAL_ACC[] ALIGNED = "l6470_kval_acc";
static const char L6470_PARAMS_KVAL_DEC[] ALIGNED = "l6470_kval_dec";
static const char L6470_PARAMS_MICRO_STEPS[] ALIGNED = "l6470_micro_steps";

L6470Params::L6470Params(const char *pFileName): m_bSetList(0) {
	assert(pFileName != 0);

    m_fMinSpeed = 0;
    m_fMaxSpeed = 0;
    m_fAcc = 0;
    m_fDec = 0;
    m_nKvalHold = 0;
    m_nKvalRun = 0;
    m_nKvalAcc = 0;
    m_nKvalDec = 0;
    m_nMicroSteps = 0;

	ReadConfigFile configfile(L6470Params::staticCallbackFunction, this);
	configfile.Read(pFileName);
}

L6470Params::~L6470Params(void) {
}

void L6470Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((L6470Params *) p)->callbackFunction(s);
}

void L6470Params::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	if (Sscan::Float(pLine, L6470_PARAMS_MIN_SPEED, &m_fMinSpeed) == SSCAN_OK) {
		m_bSetList |= SET_MIN_SPEED_MASK;
		return;
	}

	if (Sscan::Float(pLine, L6470_PARAMS_MAX_SPEED, &m_fMaxSpeed) == SSCAN_OK) {
		m_bSetList |= SET_MAX_SPEED_MASK;
		return;
	}

	if (Sscan::Float(pLine, L6470_PARAMS_ACC, &m_fAcc) == SSCAN_OK) {
		m_bSetList |= SET_ACC_MASK;
		return;
	}

	if (Sscan::Float(pLine, L6470_PARAMS_DEC, &m_fDec) == SSCAN_OK) {
		m_bSetList |= SET_DEC_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, L6470_PARAMS_KVAL_HOLD, &m_nKvalHold) == SSCAN_OK) {
		m_bSetList |= SET_KVAL_HOLD_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, L6470_PARAMS_KVAL_RUN, &m_nKvalRun) == SSCAN_OK) {
		m_bSetList |= SET_KVAL_RUN_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, L6470_PARAMS_KVAL_ACC, &m_nKvalAcc) == SSCAN_OK) {
		m_bSetList |= SET_KVAL_ACC_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, L6470_PARAMS_KVAL_DEC, &m_nKvalDec) == SSCAN_OK) {
		m_bSetList |= SET_KVAL_DEC_MASK;
		return;
	}

	if (Sscan::Uint8(pLine, L6470_PARAMS_MICRO_STEPS, &m_nMicroSteps) == SSCAN_OK) {
		m_bSetList |= SET_MICRO_STEPS_MASK;
		return;
	}
}

void L6470Params::Set(L6470 *pL6470) {
	assert(pL6470 != 0);

	if(isMaskSet(SET_MIN_SPEED_MASK)) {
		pL6470->setMinSpeed(m_fMinSpeed);
	}

	if(isMaskSet(SET_MAX_SPEED_MASK)) {
		pL6470->setMaxSpeed(m_fMaxSpeed);
	}

	if(isMaskSet(SET_ACC_MASK)) {
		pL6470->setAcc(m_fAcc);
	}

	if(isMaskSet(SET_DEC_MASK)) {
		pL6470->setDec(m_fDec);
	}

	if(isMaskSet(SET_KVAL_HOLD_MASK)) {
		pL6470->setHoldKVAL(m_nKvalHold);
	}

	if(isMaskSet(SET_KVAL_RUN_MASK)) {
		pL6470->setRunKVAL(m_nKvalRun);
	}

	if(isMaskSet(SET_KVAL_ACC_MASK)) {
		pL6470->setAccKVAL(m_nKvalAcc);
	}

	if(isMaskSet(SET_KVAL_DEC_MASK)) {
		pL6470->setDecKVAL(m_nKvalDec);
	}

	if(isMaskSet(SET_MICRO_STEPS_MASK)) {
		pL6470->setMicroSteps(m_nMicroSteps);
	}
}

void L6470Params::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(SET_MIN_SPEED_MASK)) {
		printf("%s=%f\n", L6470_PARAMS_MIN_SPEED, m_fMinSpeed);
	}

	if(isMaskSet(SET_MAX_SPEED_MASK)) {
		printf("%s=%f\n", L6470_PARAMS_MAX_SPEED, m_fMaxSpeed);
	}

	if(isMaskSet(SET_ACC_MASK)) {
		printf("%s=%f\n", L6470_PARAMS_ACC, m_fAcc);
	}

	if(isMaskSet(SET_DEC_MASK)) {
		printf("%s=%f\n", L6470_PARAMS_DEC, m_fDec);
	}

	if(isMaskSet(SET_KVAL_HOLD_MASK)) {
		printf("%s=%d\n", L6470_PARAMS_KVAL_HOLD, m_nKvalHold);
	}

	if(isMaskSet(SET_KVAL_RUN_MASK)) {
		printf("%s=%d\n", L6470_PARAMS_KVAL_RUN, m_nKvalRun);
	}

	if(isMaskSet(SET_KVAL_ACC_MASK)) {
		printf("%s=%d\n", L6470_PARAMS_KVAL_ACC, m_nKvalAcc);
	}

	if(isMaskSet(SET_KVAL_DEC_MASK)) {
		printf("%s=%d\n", L6470_PARAMS_KVAL_DEC, m_nKvalDec);
	}

	if(isMaskSet(SET_MICRO_STEPS_MASK)) {
		printf("%s=%d\n", L6470_PARAMS_MICRO_STEPS, (int) m_nMicroSteps);
	}
#endif
}

bool L6470Params::isMaskSet(uint32_t mask) const {
	return (m_bSetList & mask) == mask;
}
