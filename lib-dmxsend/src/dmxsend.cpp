/**
 * @file dmxsend.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmxsend.h"

#include "lightset.h"
#include "lightsetdata.h"

#include "dmx.h"
#include "panel_led.h"
#include "hardware.h"

#include "debug.h"

namespace dmxsend {
static constexpr bool is_started(const uint8_t v, const uint32_t p) {
	return (v & (1U << p)) == (1U << p);
}
}  // namespace dmxsend

void DmxSend::Start(const uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

	assert(nPortIndex < CHAR_BIT);

	if (dmxsend::is_started(m_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	m_nStarted = static_cast<uint8_t>(m_nStarted | (1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, true);

	if (Dmx::Get()->GetOutputStyle(nPortIndex) == dmx::OutputStyle::CONTINOUS) {
		hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
	}

	DEBUG_EXIT
}

void DmxSend::Stop(const uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%d -> %u", nPortIndex,	dmxsend::is_started(m_nStarted, static_cast<uint8_t>(nPortIndex)));

	assert(nPortIndex < CHAR_BIT);

	if (!dmxsend::is_started(m_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	m_nStarted = static_cast<uint8_t>(m_nStarted & ~(1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	hal::panel_led_off(hal::panelled::PORT_A_TX << nPortIndex);

	DEBUG_EXIT
}

void DmxSend::SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) {
	assert(nPortIndex < CHAR_BIT);
	assert(pData != nullptr);
	assert(nLength != 0);

	if (doUpdate) {
		Dmx::Get()->SetSendDataWithoutSC(nPortIndex, pData, nLength);
		Dmx::Get()->StartOutput(nPortIndex);
		hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
	}
}

void DmxSend::Sync(uint32_t const nPortIndex) {
	assert(lightset::Data::GetLength(nPortIndex) != 0);
	Dmx::Get()->SetSendDataWithoutSC(nPortIndex, lightset::Data::Backup(nPortIndex), lightset::Data::GetLength(nPortIndex));
}

void DmxSend::Sync(const bool doForce) {
	Dmx::Get()->SetOutput(doForce);

	for (uint32_t nPortIndex = 0; nPortIndex < dmx::config::max::OUT; nPortIndex++) {
		if (lightset::Data::GetLength(nPortIndex) != 0) {
			lightset::Data::ClearLength(nPortIndex);
			hal::panel_led_on(hal::panelled::PORT_A_TX << nPortIndex);
			if (!dmxsend::is_started(m_nStarted, nPortIndex)) {
				Start(nPortIndex);
			}
		}
	}
}

#include <cstdio>

void DmxSend::Print() {
	puts("DMX Send");
	printf(" Break time   : %u\n", Dmx::Get()->GetDmxBreakTime());
	printf(" MAB time     : %u\n", Dmx::Get()->GetDmxMabTime());
	printf(" Refresh rate : %u\n", 1000000U / Dmx::Get()->GetDmxPeriodTime());
	printf(" Slots        : %u\n", Dmx::Get()->GetDmxSlots());
}
