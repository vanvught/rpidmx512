/**
 * @file ws28xxdmxmulti.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#include <cassert>

#include "ws28xxdmxmulti.h"
#include "ws28xxmulti.h"
#include "ws28xxdmxparams.h"
#include "ws28xx.h"

#include "rgbmapping.h"

#include "debug.h"

using namespace ws28xxdmxmulti;
using namespace ws28xx;

static uint8_t s_StopBuffer[DMX_UNIVERSE_SIZE];

WS28xxDmxMulti::WS28xxDmxMulti() {
	DEBUG_ENTRY

	UpdateMembers();

	for (uint32_t i = 0; i < sizeof(s_StopBuffer); i++) {
		s_StopBuffer[i] = 0;
	}

	m_pLEDStripe = new WS28xxMulti;

	DEBUG_EXIT
}

WS28xxDmxMulti::~WS28xxDmxMulti() {
	delete m_pLEDStripe;
	m_pLEDStripe = nullptr;
}

void WS28xxDmxMulti::Initialize() {
	assert(m_pLEDStripe != nullptr);

	DEBUG_PRINTF("m_tLedType=%d, m_nLedCount=%d, m_tRGBMapping=%d, m_nLowCode=%d, m_nHighCode=%d", m_tLedType, m_nLedCount, static_cast<int>(m_tRGBMapping), m_nLowCode, m_nHighCode);

	m_pLEDStripe->Initialize(m_tLedType, m_nLedCount, m_tRGBMapping, m_nLowCode, m_nHighCode, m_bUseSI5351A);

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	m_pLEDStripe->Blackout();
}

void WS28xxDmxMulti::Start(uint8_t nPortId) {
	DEBUG_PRINTF("%d", static_cast<int>(nPortId));

	if (m_bIsStarted == 0) {
		if (m_pLightSetHandler != nullptr) {
			m_pLightSetHandler->Start();
		}
	}

	m_bIsStarted |= (1U << nPortId);
}

void WS28xxDmxMulti::Stop(uint8_t nPortId) {
	assert(m_pLEDStripe != nullptr);

	DEBUG_PRINTF("%d", static_cast<int>(nPortId));

	if (m_bIsStarted & (1U << nPortId)) {
		SetData(nPortId, s_StopBuffer, sizeof(s_StopBuffer));
		m_bIsStarted &= ~(1U << nPortId);
	}

	if ((m_bIsStarted == 0) & (m_pLightSetHandler != nullptr)) {
		m_pLightSetHandler->Stop();
	}
}

void WS28xxDmxMulti::SetData(uint8_t nPortId, const uint8_t* pData, uint16_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= DMX_UNIVERSE_SIZE);
	assert(m_pLEDStripe != nullptr);

	uint32_t i = 0;
	uint32_t beginIndex, endIndex;

#if defined (NODE_ARTNET)
	const uint32_t nOutIndex = nPortId / 4;
	const uint8_t nSwitch = nPortId - (nOutIndex * 4);
#else
	const uint32_t nOutIndex = nPortId / m_nUniverses;
	const uint8_t nSwitch = nPortId - (nOutIndex * m_nUniverses);
#endif

	switch (nSwitch) {
	case 0:
		beginIndex = 0;
		endIndex = std::min(m_nLedCount, (nLength / m_nChannelsPerLed));
		break;
	case 1:
		beginIndex = m_nBeginIndexPortId1;
		endIndex = std::min(m_nLedCount, (beginIndex + (nLength / m_nChannelsPerLed)));
		break;
	case 2:
		beginIndex = m_nBeginIndexPortId2;
		endIndex = std::min(m_nLedCount, (beginIndex + (nLength / m_nChannelsPerLed)));
		break;
	case 3:
		beginIndex = m_nBeginIndexPortId3;
		endIndex = std::min(m_nLedCount, (beginIndex + (nLength / m_nChannelsPerLed)));
		break;
	default:
		__builtin_unreachable();
		break;
	}

#if 0
	DEBUG_PRINTF("nPort=%d, nLength=%d, nOutIndex=%d, nSwitch=%d, beginIndex=%d, endIndex=%d",
			static_cast<int>(nPortId), static_cast<int>(nLength), static_cast<int>(nOutIndex),
			static_cast<int>(nSwitch), static_cast<int>(beginIndex), static_cast<int>(endIndex));
#endif

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t j = beginIndex; j < endIndex; j++) {
		__builtin_prefetch(&pData[i]);
		if (m_tLedType == Type::SK6812W) {
			if (i + 3 > nLength) {
				break;
			}
			m_pLEDStripe->SetLED(nOutIndex, j, pData[i], pData[i + 1], pData[i + 2], pData[i + 3]);
			i = i + 4;
		} else {
			if (i + 2 > nLength) {
				break;
			}
			m_pLEDStripe->SetLED(nOutIndex, j, pData[i], pData[i + 1], pData[i + 2]);
			i = i + 3;
		}
	}

	if (nPortId == m_nPortIdLast) {
		m_pLEDStripe->Update();
	}
}

void WS28xxDmxMulti::Blackout(bool bBlackout) {
	m_bBlackout = bBlackout;

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	if (bBlackout) {
		m_pLEDStripe->Blackout();
	} else {
		m_pLEDStripe->Update();
	}
}

void WS28xxDmxMulti::SetLEDType(Type tWS28xxMultiType) {
	DEBUG_ENTRY

	m_tLedType = tWS28xxMultiType;

	if (tWS28xxMultiType == Type::SK6812W) {
		m_nBeginIndexPortId1 = 128;
		m_nBeginIndexPortId2 = 256;
		m_nBeginIndexPortId3 = 384;

		m_nChannelsPerLed = 4;
	}

	UpdateMembers();

	DEBUG_EXIT
}

void WS28xxDmxMulti::SetLEDCount(uint16_t nLedCount) {
	DEBUG_ENTRY

	m_nLedCount = nLedCount;

	UpdateMembers();

	DEBUG_EXIT
}

void WS28xxDmxMulti::SetActivePorts(uint8_t nActiveOutputs) {
	DEBUG_ENTRY

	const uint8_t nMaxActiveOutputs = (m_pLEDStripe->GetBoard() == ws28xxmulti::Board::X4 ? 4 : 8);

	m_nActiveOutputs = std::min(nActiveOutputs, nMaxActiveOutputs);

	UpdateMembers();

	DEBUG_EXIT
}

void WS28xxDmxMulti::UpdateMembers() {
	m_nUniverses = 1 + (m_nLedCount / (1 + m_nBeginIndexPortId1));

#if defined (NODE_ARTNET)
	m_nPortIdLast = ((m_nActiveOutputs - 1) * 4) + m_nUniverses - 1;
#else
	m_nPortIdLast = (m_nActiveOutputs * m_nUniverses)  - 1;
#endif

	DEBUG_PRINTF("m_tLedType=%d, m_nLedCount=%d, m_nUniverses=%d, m_nPortIndexLast=%d", static_cast<int>(m_tLedType), static_cast<int>(m_nLedCount), static_cast<int>(m_nUniverses), static_cast<int>(m_nPortIdLast));
}

void WS28xxDmxMulti::Print() {
	assert(m_pLEDStripe != nullptr);

	printf("Led parameters\n");
	printf(" Type    : %s [%d]\n", WS28xx::GetLedTypeString(m_pLEDStripe->GetLEDType()), m_pLEDStripe->GetLEDType());
	printf(" Mapping : %s [%d]\n", RGBMapping::ToString(m_pLEDStripe->GetRgbMapping()), m_pLEDStripe->GetRgbMapping());
	printf(" T0H     : %.2f [0x%X]\n", WS28xx::ConvertTxH(m_pLEDStripe->GetLowCode()), m_pLEDStripe->GetLowCode());
	printf(" T1H     : %.2f [0x%X]\n", WS28xx::ConvertTxH(m_pLEDStripe->GetHighCode()), m_pLEDStripe->GetHighCode());
	printf(" Count   : %d\n", m_nLedCount);
	printf(" Outputs : %d\n", m_nActiveOutputs);
	printf(" Board   : %dx\n", m_pLEDStripe->GetBoard() == ws28xxmulti::Board::X4 ? 4 : 8);
	if (m_pLEDStripe->GetBoard() == ws28xxmulti::Board::X4) {
		printf("  SI5351A : %c\n", m_bUseSI5351A ? 'Y' : 'N');
	}
}

void WS28xxDmxMulti::SetTestPattern(pixelpatterns::Pattern TestPattern) {
	if ((TestPattern != pixelpatterns::Pattern::NONE) && (TestPattern < pixelpatterns::Pattern::LAST)) {
		if (m_pPixelPatterns == nullptr) {
			m_pPixelPatterns = new PixelPatterns(m_nActiveOutputs);
			assert(m_pPixelPatterns != nullptr);
		}

		const auto nColour1 = m_pPixelPatterns->Colour(0, 0, 0);
		const auto nColour2 = m_pPixelPatterns->Colour(100, 100, 100);
		constexpr auto nInterval = 100;
		constexpr auto nSteps = 10;

		for (uint32_t i = 0; i < m_nActiveOutputs; i++) {
			switch (TestPattern) {
			case pixelpatterns::Pattern::RAINBOW_CYCLE:
				m_pPixelPatterns->RainbowCycle(i, nInterval);
				break;
			case pixelpatterns::Pattern::THEATER_CHASE:
				m_pPixelPatterns->TheaterChase(i, nColour1, nColour2, nInterval);
				break;
			case pixelpatterns::Pattern::COLOR_WIPE:
				m_pPixelPatterns->ColourWipe(i, nColour2, nInterval);
				break;
			case pixelpatterns::Pattern::SCANNER:
				m_pPixelPatterns->Scanner(i, m_pPixelPatterns->Colour(255, 255, 255), nInterval);
				break;
			case pixelpatterns::Pattern::FADE:
				m_pPixelPatterns->Fade(i, nColour1, nColour2, nSteps, nInterval);
				break;
			default:
				break;
			}
		}
	}
}

void WS28xxDmxMulti::RunTestPattern() {
	if (m_pPixelPatterns != nullptr) {
		m_pPixelPatterns->Run();
	}
}
