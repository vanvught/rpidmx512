/**
 * @file artnetdmxinput.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmx/artnetdmxinput.h"
#include "dmx.h"

#include "debug.h"

uint8_t ArtNetDmxInput::s_nStarted;

static constexpr bool is_started(const uint8_t v, const uint32_t p) {
	return (v & (1U << p)) == (1U << p);
}

ArtNetDmxInput::ArtNetDmxInput() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ArtNetDmxInput::Start(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(Dmx::Get() != nullptr);
	assert(nPortIndex < CHAR_BIT);

	DEBUG_PRINTF("nPortIndex=%d", nPortIndex);

	if (is_started(s_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	s_nStarted = static_cast<uint8_t>(s_nStarted | (1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);

	DEBUG_EXIT
}

void ArtNetDmxInput::Stop(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < CHAR_BIT);

	DEBUG_PRINTF("nPortIndex=%d -> %u", nPortIndex, is_started(s_nStarted, nPortIndex));

	if (!is_started(s_nStarted, nPortIndex)) {
		DEBUG_EXIT
		return;
	}

	s_nStarted = static_cast<uint8_t>(s_nStarted & ~(1U << nPortIndex));

	Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, false);

	DEBUG_EXIT
}

const uint8_t *ArtNetDmxInput::Handler(uint32_t nPortIndex, uint32_t& nLength, uint32_t& nUpdatesPerSecond) {
	assert(nPortIndex < CHAR_BIT);

	const auto *pDmx = Dmx::Get()->GetDmxAvailable(nPortIndex);

	nUpdatesPerSecond = Dmx::Get()->GetUpdatesPerSecond(nPortIndex);

	if (pDmx != nullptr) {
		const auto *pDmxData = reinterpret_cast<const struct Data*>(pDmx);
		nLength = pDmxData->Statistics.nSlotsInPacket;
		return (pDmx + 1);
	}

	nLength = 0;
	return nullptr;
}
