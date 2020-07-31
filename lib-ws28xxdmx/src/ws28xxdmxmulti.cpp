/**
 * @file ws28xxdmxmulti.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

WS28xxDmxMulti::WS28xxDmxMulti(TWS28xxDmxMultiSrc tSrc):
	m_tSrc(tSrc),
	m_tLedType(WS2812B),
	m_tRGBMapping(RGB_MAPPING_UNDEFINED),
	m_nLowCode(0),
	m_nHighCode(0),
	m_nLedCount(170),
	m_nActiveOutputs(1),
	m_pLEDStripe(nullptr),
	m_bIsStarted(false),
	m_bBlackout(false),
	m_nUniverses(1), // -> m_nLedCount(170)
	m_nBeginIndexPortId1(170),
	m_nBeginIndexPortId2(340),
	m_nBeginIndexPortId3(510),
	m_nChannelsPerLed(3),
	m_nPortIdLast(3), // -> (m_nActiveOutputs * m_nUniverses) -1;
	m_bUseSI5351A(false)
{
	DEBUG_ENTRY

	UpdateMembers();

	m_pLEDStripe = new WS28xxMulti;

	DEBUG_EXIT
}

WS28xxDmxMulti::~WS28xxDmxMulti() {
	delete m_pLEDStripe;
	m_pLEDStripe = nullptr;
}

void WS28xxDmxMulti::Initialize() {
	assert(m_pLEDStripe != nullptr);

	m_pLEDStripe->Initialize(m_tLedType, m_nLedCount, m_tRGBMapping, m_nLowCode, m_nHighCode, m_bUseSI5351A);

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	m_pLEDStripe->Blackout();
}

void WS28xxDmxMulti::Start(__attribute__((unused)) uint8_t nPort) {
	assert(m_pLEDStripe != nullptr);

	DEBUG_PRINTF("%d", static_cast<int>(nPort));

	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	m_pLEDStripe->Update();
}

void WS28xxDmxMulti::Stop(__attribute__((unused)) uint8_t nPort) {
	assert(m_pLEDStripe != nullptr);

	DEBUG_PRINTF("%d", static_cast<int>(nPort));

	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	m_pLEDStripe->Blackout();
}

void WS28xxDmxMulti::SetData(uint8_t nPortId, const uint8_t* pData, uint16_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= DMX_UNIVERSE_SIZE);
	assert(m_pLEDStripe != nullptr);

	uint32_t i = 0;
	uint32_t beginIndex, endIndex;

	switch (nPortId & ~static_cast<uint8_t>(m_nUniverses) & 0x03) {
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

	uint32_t nOutIndex;

	if (m_tSrc == WS28XXDMXMULTI_SRC_E131) {
		nOutIndex = nPortId / m_nUniverses;
	} else {
		nOutIndex = nPortId / 4;
	}

	DEBUG_PRINTF("nPort=%d, nLength=%d, nOutIndex=%d, nPortId=%d, beginIndex=%d, endIndex=%d",
			static_cast<int>(nPortId), static_cast<int>(nLength), static_cast<int>(nOutIndex),
			static_cast<int>(nPortId & ~m_nUniverses & 0x03), static_cast<int>(beginIndex), static_cast<int>(endIndex));

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t j = beginIndex; j < endIndex; j++) {
		__builtin_prefetch(&pData[i]);
		if (m_tLedType == SK6812W) {
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

void WS28xxDmxMulti::SetLEDType(TWS28XXType tWS28xxMultiType) {
	DEBUG_ENTRY

	m_tLedType = tWS28xxMultiType;

	if (tWS28xxMultiType == SK6812W) {
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

	const uint8_t nMaxActiveOutputs = (m_pLEDStripe->GetBoard() == WS28XXMULTI_BOARD_4X ? 4 : 8);

	m_nActiveOutputs = std::min(nActiveOutputs, nMaxActiveOutputs);

	UpdateMembers();

	DEBUG_EXIT
}

void WS28xxDmxMulti::UpdateMembers() {
	m_nUniverses = 1 + (m_nLedCount / (1 + m_nBeginIndexPortId1));

	if (m_tSrc == WS28XXDMXMULTI_SRC_E131) {
		m_nPortIdLast = (m_nActiveOutputs * m_nUniverses)  - 1;
	} else {
		m_nPortIdLast = ((m_nActiveOutputs - 1) * 4) + m_nUniverses - 1;
	}

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
	printf(" Board   : %dx\n", m_pLEDStripe->GetBoard() == WS28XXMULTI_BOARD_4X ? 4 : 8);
	if (m_pLEDStripe->GetBoard() == WS28XXMULTI_BOARD_4X) {
		printf("  SI5351A : %c\n", m_bUseSI5351A ? 'Y' : 'N');
	}
}
