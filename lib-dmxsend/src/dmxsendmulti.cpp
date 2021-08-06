/**
 * @file dmxsendmulti.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmxsendmulti.h"

#include "debug.h"

using namespace dmxmulti;

DmxSendMulti::DmxSendMulti() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < max::OUT ; i++) {
		m_bIsStarted[i] = false;
	}

	DEBUG_EXIT
}

void DmxSendMulti::Start(uint32_t nPortIndex) {
	DEBUG_ENTRY

	assert(nPortIndex < max::OUT);

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

	if (m_bIsStarted[nPortIndex]) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted[nPortIndex] = true;

	SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, true);

	DEBUG_EXIT
}

void DmxSendMulti::Stop(uint32_t nPortIndex) {
	DEBUG_ENTRY

	assert(nPortIndex < max::OUT);

	DEBUG_PRINTF("nPortIndex=%d -> %u", nPortIndex, m_bIsStarted[nPortIndex]);

	if (!m_bIsStarted[nPortIndex]) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted[nPortIndex] = false;

	SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	DEBUG_EXIT
}

void DmxSendMulti::SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex < max::OUT);
	assert(pData != nullptr);

	if (__builtin_expect((nLength == 0), 0)) {
		return;
	}

	SetPortSendDataWithoutSC(nPortIndex, pData, nLength);
}
