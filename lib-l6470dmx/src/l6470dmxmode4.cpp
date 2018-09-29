/**
 * @file l6470dmxmode4.cpp
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

#include "l6470dmxmode4.h"
#include "l6470.h"

#include "motorparams.h"

#include "debug.h"

L6470DmxMode4::L6470DmxMode4(L6470 *pL6470, MotorParams *pMotorParams, ModeParams *pModeParams): m_nPreviousData(0), m_bWasBusy(false) {
	DEBUG2_ENTRY;

	assert(pL6470 != 0);
	assert(pMotorParams != 0);
	assert(pModeParams != 0);

	m_pModeParams = pModeParams;
	m_pL6470 = pL6470;
	m_fSteps = (float) pModeParams->GetMaxSteps() / 0xFF;

	DEBUG2_EXIT;
}

L6470DmxMode4::~L6470DmxMode4(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode4::InitSwitch(void) {
	DEBUG2_ENTRY;

	if (m_pModeParams->HasSwitch()) {
		const TL6470Action action = m_pModeParams->GetSwitchAction();
		const TL6470Direction dir = m_pModeParams->GetSwitchDir();
		const float stepsPerSec = m_pModeParams->GetSwitchStepsPerSec();

		m_pL6470->goUntil(action, dir, stepsPerSec);
	}

	DEBUG2_EXIT;
}

void L6470DmxMode4::InitPos(void) {
	DEBUG2_ENTRY;

	m_pL6470->resetPos();

	DEBUG2_EXIT;
}

void L6470DmxMode4::Start(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode4::Stop(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode4::HandleBusy(void) {
	DEBUG2_ENTRY

	if (m_pL6470->busyCheck()) {
#ifndef NDEBUG
		printf("\t\t\tBusy!\n");
#endif
		m_pL6470->softStop();
		m_bWasBusy = true;
	} else {
		m_bWasBusy = false;
	}

	DEBUG2_EXIT
}

bool L6470DmxMode4::BusyCheck(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
	return m_pL6470->busyCheck();
}

void L6470DmxMode4::Data(const uint8_t *pDmxData) {
	DEBUG2_ENTRY;

	const uint32_t steps = pDmxData[0] * m_fSteps;
	bool isRev;
#ifndef NDEBUG
	int32_t nDifference;
#endif

	if(m_bWasBusy) {
		uint32_t nCurrentPosition = m_pL6470->getPos();
		isRev = nCurrentPosition > steps;
#ifndef NDEBUG
		printf("\t\t\tCurrent position=%d\n", nCurrentPosition);
		nDifference = (uint64_t) steps - nCurrentPosition;
#endif
	} else {
		isRev = m_nPreviousData > pDmxData[0];
#ifndef NDEBUG
		nDifference = (uint16_t) pDmxData[0] - m_nPreviousData;
#endif
	}

#ifndef NDEBUG
	printf("\t\t\tm_fSteps=%f, steps=%d, pDmxData[0]=%d, nDifference=%d [%s]\n", m_fSteps, (int) steps, pDmxData[0], nDifference, isRev ? "L6470_DIR_REV" : "L6470_DIR_FWD" );
#endif

	m_pL6470->goToDir(isRev ? L6470_DIR_REV : L6470_DIR_FWD, steps);

	m_nPreviousData = pDmxData[0];

	DEBUG2_EXIT;
}
