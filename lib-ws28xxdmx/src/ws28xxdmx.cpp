/**
 * @file ws28xxdmx.cpp
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#include <cassert>

#ifndef NDEBUG
#if (__linux__)
 #include <stdio.h>
#endif
#endif

#include "ws28xxdmx.h"
#include "ws28xx.h"

#include "lightset.h"
#include "lightsetdisplay.h"

WS28xxDmx::WS28xxDmx() :
	
	m_nDmxFootprint(170 * 3)
	
{
	UpdateMembers();
}

WS28xxDmx::~WS28xxDmx() {
	delete m_pLEDStripe;
	m_pLEDStripe = nullptr;
}

void WS28xxDmx::Start(__attribute__((unused)) uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pLEDStripe == nullptr) {
		m_pLEDStripe = new WS28xx(m_tLedType, m_nLedCount, m_tRGBMapping, m_nLowCode, m_nHighCode, m_nClockSpeedHz);
		assert(m_pLEDStripe != nullptr);
		m_pLEDStripe->SetGlobalBrightness(m_nGlobalBrightness);
		m_pLEDStripe->Initialize();
	} else {
		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}
		m_pLEDStripe->Update();
	}
}

void WS28xxDmx::Stop(__attribute__((unused)) uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_pLEDStripe != nullptr) {
		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}
		m_pLEDStripe->Blackout();
	}
}

void WS28xxDmx::SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= DMX_UNIVERSE_SIZE);

	uint32_t i = 0;
	uint32_t beginIndex, endIndex;

	if (__builtin_expect((m_pLEDStripe == nullptr), 0)) {
		m_bIsStarted = false;
		Start();
	}

	switch (nPortId & 0x03) {
	case 0:
		beginIndex = 0;
		endIndex = std::min(m_nLedCount, static_cast<uint16_t>(nLength / m_nChannelsPerLed));
		if (m_nLedCount < m_nBeginIndexPortId1) {
			i = static_cast<uint32_t>(m_nDmxStartAddress - 1);
		}
		break;
	case 1:
		beginIndex = m_nBeginIndexPortId1;
		endIndex = std::min(m_nLedCount, static_cast<uint16_t>(beginIndex + (nLength /  m_nChannelsPerLed)));
		break;
	case 2:
		beginIndex = m_nBeginIndexPortId2;
		endIndex = std::min(m_nLedCount, static_cast<uint16_t>(beginIndex + (nLength /  m_nChannelsPerLed)));
		break;
	case 3:
		beginIndex = m_nBeginIndexPortId3;
		endIndex = std::min(m_nLedCount, static_cast<uint16_t>(beginIndex + (nLength /  m_nChannelsPerLed)));
		break;
	default:
		__builtin_unreachable();
		break;
	}

#ifndef NDEBUG
#if defined (__linux__)
	printf("%d-%d:%x %x %x-%d", nPortId, m_nDmxStartAddress, pData[0], pData[1], pData[2], nLength);
#endif
#endif

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t j = beginIndex; j < endIndex; j++) {
		__builtin_prefetch(&pData[i]);
		if (m_tLedType == SK6812W) {
			if (i + 3 > nLength) {
				break;
			}
			m_pLEDStripe->SetLED(j, pData[i], pData[i + 1], pData[i + 2], pData[i + 3]);
			i = i + 4;
		} else {
			if (i + 2 > nLength) {
				break;
			}
			m_pLEDStripe->SetLED(j, pData[i], pData[i + 1], pData[i + 2]);
			i = i + 3;
		}
	}

	if (nPortId == m_nPortIdLast) {
		m_pLEDStripe->Update();
	}
}

void WS28xxDmx::SetLEDType(TWS28XXType type) {
	m_tLedType = type;

	if (type == SK6812W) {
		m_nBeginIndexPortId1 = 128;
		m_nBeginIndexPortId2 = 256;
		m_nBeginIndexPortId3 = 384;

		m_nChannelsPerLed = 4;
	}

	UpdateMembers();
}

void WS28xxDmx::SetLEDCount(uint16_t nCount) {
	m_nLedCount = nCount;

	UpdateMembers();
}

void WS28xxDmx::UpdateMembers() {
	m_nDmxFootprint = m_nLedCount * m_nChannelsPerLed;

	if (m_nDmxFootprint > DMX_UNIVERSE_SIZE) {
		m_nDmxFootprint = DMX_UNIVERSE_SIZE;
	}

	m_nPortIdLast = m_nLedCount / (1 + m_nBeginIndexPortId1);
}

void WS28xxDmx::Blackout(bool bBlackout) {
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

// DMX

bool WS28xxDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_UNIVERSE_SIZE));

	//FIXME Footprint

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_UNIVERSE_SIZE)) {
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

// RDM

#define MOD(a,b)	(static_cast<unsigned>(a) - b * (static_cast<unsigned>(a)/b))

bool WS28xxDmx::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	unsigned nIndex;

	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	if (m_tLedType == SK6812W) {
		nIndex = MOD(nSlotOffset, 4);
	} else {
		nIndex = MOD(nSlotOffset, 3);
	}

	tSlotInfo.nType = 0x00;	// ST_PRIMARY

	switch (nIndex) {
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
