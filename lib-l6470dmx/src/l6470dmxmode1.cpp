/**
 * @file l6470dmxmode1.cpp
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
#include <assert.h>

#include "l6470dmxmode1.h"
#include "l6470.h"

#include "motorparams.h"

#include "debug.h"

L6470DmxMode1::L6470DmxMode1(L6470 *pL6470, MotorParams *pMotorParams) {
	DEBUG2_ENTRY;

	assert(pL6470 != 0);
	assert(pMotorParams != 0);

	m_pL6470 = pL6470;

	m_fMinSpeed = m_pL6470->getMinSpeed();
	m_fMaxSpeed = m_pL6470->getMaxSpeed();

	DEBUG2_EXIT;
}

L6470DmxMode1::~L6470DmxMode1(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode1::Start(void) {
	DEBUG2_ENTRY;

	DEBUG2_EXIT;
}

void L6470DmxMode1::Stop(void) {
	DEBUG2_ENTRY;

	m_pL6470->hardHiZ();

	DEBUG2_EXIT;
}

void L6470DmxMode1::Data(const uint8_t *pDmxData) {
	DEBUG2_ENTRY;

	float speed = m_fMinSpeed + (float) pDmxData[0] * ((m_fMaxSpeed - m_fMinSpeed) / 255);

	if (pDmxData[1] <= 127) {	// Right-hand rotation
		m_pL6470->run(L6470_DIR_REV, speed);
	} else {					// Left-hand rotation
		m_pL6470->run(L6470_DIR_FWD, speed);
	}

	DEBUG2_EXIT;
}
