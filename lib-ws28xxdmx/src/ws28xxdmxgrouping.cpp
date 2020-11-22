/**
 * @file ws28xxstripedmxgrouping.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ws28xxdmxgrouping.h"
#include "ws28xxdmxparams.h"
#include "ws28xx.h"

#include "lightset.h"
#include "lightsetdisplay.h"

#include "debug.h"

WS28xxDmxGrouping::WS28xxDmxGrouping()
	
{
	UpdateMembers();
}

WS28xxDmxGrouping::~WS28xxDmxGrouping() {
	delete [] m_pDmxData;
	m_pDmxData = nullptr;
}

void WS28xxDmxGrouping::Start(__attribute__((unused)) uint8_t nPort) {
	m_pDmxData = new uint8_t[m_nDmxFootprint];
	assert(m_pDmxData != nullptr);

	WS28xxDmx::Start();
}

void WS28xxDmxGrouping::SetData(__attribute__((unused)) uint8_t nPort, const uint8_t* pData, uint16_t nLength) {
	if (__builtin_expect((m_pLEDStripe == nullptr), 0)) {
		Start();
	}

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	bool bIsChanged = false;

	for (uint32_t i = static_cast<uint32_t>(m_nDmxStartAddress - 1), j = 0; (i < nLength) && (j < m_nDmxFootprint); i++, j++) {
		if (pData[i] != m_pDmxData[j]) {
			m_pDmxData[j] = pData[i];
			bIsChanged = true;
		}
	}

	if (bIsChanged) {
		uint32_t i = 0;
		uint32_t d = 0;

		if (m_tLedType == SK6812W) {
			for (uint32_t g = 0; g < m_nGroups; g++) {
				__builtin_prefetch(&m_pDmxData[d]);
				for (uint32_t k = 0; k < m_nLEDGroupCount; k++) {
					m_pLEDStripe->SetLED(k + i, m_pDmxData[d + 0], m_pDmxData[d + 1], m_pDmxData[d + 2], m_pDmxData[d + 3]);
				}
				i = i + m_nLEDGroupCount;
				d = d + 4;
			}
		} else {
			for (uint32_t g = 0; g < m_nGroups; g++) {
				__builtin_prefetch(&m_pDmxData[d]);
				for (uint32_t k = 0; k < m_nLEDGroupCount; k++) {
					m_pLEDStripe->SetLED(k + i, m_pDmxData[d + 0], m_pDmxData[d + 1], m_pDmxData[d + 2]);
				}
				i = i + m_nLEDGroupCount;
				d = d + 3;
			}
		}

		if (!m_bBlackout) {
			m_pLEDStripe->Update();
		}
	}
}

void WS28xxDmxGrouping::SetLEDType(TWS28XXType tLedType) {
	DEBUG_PRINTF("tLedType=%d", static_cast<int>(tLedType));

	m_tLedType = tLedType;

	UpdateMembers();
}

void WS28xxDmxGrouping::SetLEDCount(uint16_t nLedCount) {
	DEBUG_PRINTF("nLedCount=%d", static_cast<int>(nLedCount));

	m_nLedCount = nLedCount;

	UpdateMembers();
}

void WS28xxDmxGrouping::SetLEDGroupCount(uint16_t nLedGroupCount) {
	DEBUG_PRINTF("nLedGroupCount=%d", static_cast<int>(nLedGroupCount));

	m_nLEDGroupCount = nLedGroupCount;

	UpdateMembers();
}

bool WS28xxDmxGrouping::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_PRINTF("nDmxStartAddress=%d", static_cast<int>(nDmxStartAddress));

	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= (DMX_UNIVERSE_SIZE - m_nDmxFootprint)));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= (DMX_UNIVERSE_SIZE - m_nDmxFootprint))) {
		m_nDmxStartAddress = nDmxStartAddress;

		if (m_pWS28xxDmxStore != nullptr) {
			m_pWS28xxDmxStore->SaveDmxStartAddress(m_nDmxStartAddress);
		}

		if (m_pLightSetDisplay != nullptr) {
			m_pLightSetDisplay->ShowDmxStartAddress();
		}

		return true;
	}

	return false;
}

void WS28xxDmxGrouping::UpdateMembers() {
	if ((m_nLEDGroupCount > m_nLedCount) || (m_nLEDGroupCount == 0)) {
		m_nLEDGroupCount = m_nLedCount;
	}

	m_nGroups = m_nLedCount / m_nLEDGroupCount;

	if (m_tLedType == SK6812W) {
		if (m_nGroups > (DMX_UNIVERSE_SIZE / 4)) {
			m_nGroups = DMX_UNIVERSE_SIZE / 4;
		}
		m_nDmxFootprint = m_nGroups * 4;
	} else {
		if (m_nGroups > (DMX_UNIVERSE_SIZE / 3)) {
			m_nGroups = DMX_UNIVERSE_SIZE / 3;
		}
		m_nDmxFootprint = m_nGroups * 3;
	}

	DEBUG_PRINTF("m_nLEDGroupCount=%d, m_nGroups=%d, m_nDmxFootprint=%d", static_cast<int>(m_nLEDGroupCount), static_cast<int>(m_nGroups), static_cast<int>(m_nDmxFootprint));
}

void WS28xxDmxGrouping::Print() {
	printf("Led (grouping) parameters\n");
	printf(" Type  : %s [%d]\n", WS28xx::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Count : %d\n", static_cast<int>(m_nLedCount));
	printf(" Group : %d\n", static_cast<int>(m_nLEDGroupCount));
}

// RDM

bool WS28xxDmxGrouping::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	tSlotInfo.nType = 0x00;	// ST_PRIMARY

	switch (nSlotOffset) {
		case 0:
			tSlotInfo.nCategory = 0x0205; // SD_COLOR_ADD_RED
			break;
		case 1:
			tSlotInfo.nCategory = 0x0206; // SD_COLOR_ADD_GREEN
			break;
		case 2:
			tSlotInfo.nCategory = 0x0207; // SD_COLOR_ADD_BLUE
			break;
		case 3:
			tSlotInfo.nCategory = 0x0212; // SD_COLOR_ADD_WHITE
			break;
		default:
			break;
	}

	return true;
}

