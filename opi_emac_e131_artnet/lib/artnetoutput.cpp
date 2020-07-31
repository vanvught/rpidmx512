/**
 * @file artnetoutput.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetoutput.h"

#include "e131bridge.h"
#include "artnetcontroller.h"

#include "debug.h"

ArtNetOutput::ArtNetOutput() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		m_nUniverse[i] = 0;
	}

	DEBUG_EXIT
}

void ArtNetOutput::Handler() {
	DEBUG_ENTRY

	ArtNetController::Get()->HandleSync();

	DEBUG_EXIT
}

void ArtNetOutput::Start(uint8_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%d", static_cast<int>(nPortIndex));

	if (nPortIndex < E131_MAX_PORTS) {
		uint16_t nUniverse;

		if (E131Bridge::Get()->GetUniverse(nPortIndex, nUniverse, E131_OUTPUT_PORT)) {
			m_nUniverse[nPortIndex] = nUniverse;
			DEBUG_PRINTF("m_nUniverse[%d]=%d", static_cast<int>(nPortIndex), static_cast<int>(m_nUniverse[nPortIndex]));
		}
	}

	DEBUG_EXIT
}

void ArtNetOutput::Stop(uint8_t nPortIndex) {
	DEBUG_ENTRY

	if (nPortIndex < E131_MAX_PORTS) {
		uint16_t nUniverse;

		if (E131Bridge::Get()->GetUniverse(nPortIndex, nUniverse, E131_OUTPUT_PORT)) {
			m_nUniverse[nPortIndex] = 0;
			DEBUG_PRINTF("m_nUniverse[%d]=0", static_cast<int>(nPortIndex));
		}
	}

	DEBUG_EXIT
}

void ArtNetOutput::SetData(uint8_t nPortIndex, const uint8_t *pDmxData, uint16_t nLength) {
	assert(nPortIndex < E131_MAX_PORTS);

	if (m_nUniverse[nPortIndex] != 0) {
		ArtNetController::Get()->HandleDmxOut(m_nUniverse[nPortIndex], pDmxData, nLength, nPortIndex);
	}
}

void ArtNetOutput::Print() {

}
