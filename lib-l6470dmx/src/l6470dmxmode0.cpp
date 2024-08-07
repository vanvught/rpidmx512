/**
 * @file l6470dmxmode0.cpp
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

#include <cstdint>
#include <cassert>

#include "l6470dmxmode0.h"
#include "l6470.h"

#include "motorparams.h"

#include "debug.h"

L6470DmxMode0::L6470DmxMode0(L6470 *pL6470) {
	DEBUG_ENTRY;

	assert(pL6470 != nullptr);

	m_pL6470 = pL6470;

	m_fMinSpeed = m_pL6470->getMinSpeed();
	m_fMaxSpeed = m_pL6470->getMaxSpeed();

	DEBUG_EXIT;
}

L6470DmxMode0::~L6470DmxMode0() {
	DEBUG_ENTRY;

	DEBUG_EXIT;
}

void L6470DmxMode0::Start() {
	DEBUG_ENTRY;

	m_pL6470->run(L6470_DIR_FWD, m_fMaxSpeed);

	DEBUG_EXIT;
}

void L6470DmxMode0::Stop() {
	DEBUG_ENTRY;

	m_pL6470->hardHiZ();

	DEBUG_EXIT;
}

void L6470DmxMode0::Data(const uint8_t *pDmxData) {
	DEBUG_ENTRY;

	if (pDmxData[0] <= 126) {	// Left-hand rotation
		m_pL6470->run(L6470_DIR_FWD, m_fMinSpeed + static_cast<float>((127 - pDmxData[0])) * ((m_fMaxSpeed - m_fMinSpeed) / 127));
		return;
	}

	if (pDmxData[0] >= 130) {	// Right-hand rotation
		m_pL6470->run(L6470_DIR_REV, m_fMinSpeed + static_cast<float>((pDmxData[0] - 129)) * ((m_fMaxSpeed - m_fMinSpeed) / 127));
		return;
	}

	m_pL6470->softStop();

	DEBUG_EXIT;
}
