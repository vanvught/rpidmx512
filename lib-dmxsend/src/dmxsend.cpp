/**
 * @file dmxsend.cpp
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
#include <climits>
#include <cassert>

#include "dmxsend.h"
#include "dmx.h"

#include "panel_led.h"

#include "debug.h"

uint8_t DmxSend::s_nStarted;

static constexpr bool is_started(const uint8_t v, const uint32_t p) {
	return (v & (1U << p)) == (1U << p);
}

void DmxSend::Start(uint32_t nPortIndex) {
	DEBUG_ENTRY

	assert(nPortIndex < CHAR_BIT);

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

	if (is_started(s_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	s_nStarted = static_cast<uint8_t>(s_nStarted | (1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, true);

	hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);

	DEBUG_EXIT
}

void DmxSend::Stop(uint32_t nPortIndex) {
	DEBUG_ENTRY

	assert(nPortIndex < CHAR_BIT);

	DEBUG_PRINTF("nPortIndex=%d -> %u", nPortIndex, is_started(s_nStarted, static_cast<uint8_t>(nPortIndex)));

	if (!is_started(s_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	s_nStarted = static_cast<uint8_t>(s_nStarted & ~(1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	hal::panel_led_off(hal::panelled::PORT_A_TX << nPortIndex);

	DEBUG_EXIT
}

void DmxSend::SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex < CHAR_BIT);
	assert(pData != nullptr);

	if (__builtin_expect((nLength == 0), 0)) {
		return;
	}

	Dmx::Get()->SetPortSendDataWithoutSC(nPortIndex, pData, nLength);
}

#include <cstdio>

void DmxSend::Print() {
	printf("DMX Send\n");
	printf(" Break time   : %d\n", static_cast<int>(Dmx::Get()->GetDmxBreakTime()));
	printf(" MAB time     : %d\n", static_cast<int>(Dmx::Get()->GetDmxMabTime()));
	printf(" Refresh rate : %d\n", static_cast<int>(1000000 / Dmx::Get()->GetDmxPeriodTime()));
	printf(" Slots        : %d\n", static_cast<int>(Dmx::Get()->GetDmxSlots()));
}
