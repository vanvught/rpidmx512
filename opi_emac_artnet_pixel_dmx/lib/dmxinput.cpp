/**
 * @file dmxinput.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>
#include <assert.h>

#include "dmxinput.h"
#include "dmx.h"

#include "debug.h"

using namespace dmx;
using namespace dmxsingle;

DmxInput::DmxInput() {
	DEBUG_ENTRY

	Stop(0);

	DEBUG_EXIT
}

void DmxInput::Start(__attribute__((unused)) uint32_t nPort) {
	DEBUG_ENTRY

	if (m_bIsStarted) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted = true;

	SetPortDirection(0, PortDirection::INP, true);
	DEBUG_EXIT
}

void DmxInput::Stop(__attribute__((unused)) uint32_t nPort) {
	DEBUG_ENTRY

	if (!m_bIsStarted) {
		DEBUG_EXIT
		return;
	}

	m_bIsStarted = false;

	SetPortDirection(0, PortDirection::INP, false);
	DEBUG_EXIT
}

const uint8_t *DmxInput::Handler(__attribute__((unused)) uint32_t nPort, uint32_t& nLength, uint32_t &nUpdatesPerSecond) {
	const auto *pDmx = GetDmxAvailable();

	nUpdatesPerSecond = GetUpdatesPerSecond();

	if (pDmx != nullptr) {
		const auto *dmx_statistics = reinterpret_cast<const struct Data*>(pDmx);
		nLength = dmx_statistics->Statistics.nSlotsInPacket;
		return (pDmx + 1);
	}

	nLength = 0;
	return nullptr;
}