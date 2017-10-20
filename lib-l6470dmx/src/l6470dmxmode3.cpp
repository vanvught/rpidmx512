/**
 * @file l6470dmxmode3.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <assert.h>

#include "l6470dmxmode3.h"
#include "l6470.h"

#include "motorparams.h"

#include "debug.h"

L6470DmxMode3::L6470DmxMode3(L6470 *pL6470, MotorParams *pMotorParams): m_nPreviousData(0) {
	DEBUG2_ENTRY;

	m_pL6470 = pL6470;

	m_nSteps = ((float) 360 / pMotorParams->getStepAngel()) * (float)(1 << pL6470->getStepMode());

	//TODO Go to home with limit switch

	DEBUG2_EXIT;
}

L6470DmxMode3::~L6470DmxMode3(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode3::Start(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode3::Stop(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode3::Data(const uint8_t *pDmxData) {
	DEBUG2_ENTRY;

	uint32_t steps = ((float) pDmxData[0] / 255) * m_nSteps;

	if (m_pL6470->busyCheck()) {
		m_pL6470->softStop();
		while(m_pL6470->busyCheck());
	}

#ifndef NDEBUG
	printf("\tm_nSteps=%d, steps=%d - pDmxData[0](%d) < m_nPreviousData(%d) = %s\n", (int) m_nSteps, (int) steps, pDmxData[0], m_nPreviousData, (pDmxData[0] < m_nPreviousData) ? "Yes" : "No" );
#endif

	if (pDmxData[0] < m_nPreviousData) {
		m_pL6470->goToDir(L6470_DIR_REV, steps);
	} else {
		m_pL6470->goToDir(L6470_DIR_FWD, steps);
	}

	m_nPreviousData = pDmxData[0];

	DEBUG2_EXIT;
}
