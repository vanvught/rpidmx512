/**
 * @file spisend.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#if defined (__circle__)
 #include <circle/interrupt.h>
#endif

#ifndef NDEBUG
#if defined (__circle__)
 #include <circle/logger.h>
#elif (__linux__)
#else
 #include "monitor.h"
#endif
#endif

#include "ws28xxstripedmx.h"
#include "ws28xxstripe.h"

#define DMX_MAX_CHANNELS	512

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MOD(a,b)	((unsigned)a - b * ((unsigned)a/b))

#if defined (__circle__)
SPISend::SPISend(CInterruptSystem *pInterruptSystem) :
	m_pInterrupt (pInterruptSystem),
#else
SPISend::SPISend(void) :
#endif
	m_nDmxStartAddress(1),
	m_nDmxFootprint(170 * 3),
	m_bIsStarted(false),
	m_pLEDStripe(0),
	m_LEDType(WS2801),
	m_nLEDCount(170),
	m_nBeginIndexPortId1(170),
	m_nBeginIndexPortId2(340),
	m_nBeginIndexPortId3(510),
	m_nChannelsPerLed(3) {
}

SPISend::~SPISend(void) {
	Stop();
	delete m_pLEDStripe;
	m_pLEDStripe = 0;
}

void SPISend::Start(uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pLEDStripe == 0) {
#if defined (__circle__)
		m_pLEDStripe = new WS28XXStripe(m_pInterrupt, m_LEDType, m_nLEDCount);
#else
		m_pLEDStripe = new WS28XXStripe(m_LEDType, m_nLEDCount);
#endif
		assert(m_pLEDStripe != 0);
		m_pLEDStripe->Initialize();
	} else {
		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}
		m_pLEDStripe->Update();
	}
}

void SPISend::Stop(uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_pLEDStripe != 0) {
		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}
		m_pLEDStripe->Blackout();
	}
}

void SPISend::SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	assert(pData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	uint16_t i = (uint16_t) 0;
	uint16_t beginIndex, endIndex;

	bool bUpdate = false;

	if (__builtin_expect((m_pLEDStripe == 0), 0)) {
		Start();
	}

	switch (nPortId) {
	case 0:
		beginIndex = (uint16_t) 0;
		endIndex = MIN(m_nLEDCount, (uint16_t) (nLength / (uint16_t) m_nChannelsPerLed));
		bUpdate = (endIndex == m_nLEDCount);
		if (m_nLEDCount < m_nBeginIndexPortId1) {
			i = m_nDmxStartAddress - 1;
		}
		break;
	case 1:
		beginIndex = (uint16_t) m_nBeginIndexPortId1;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (nLength / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 2:
		beginIndex = (uint16_t) m_nBeginIndexPortId2;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (nLength / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 3:
		beginIndex = (uint16_t) m_nBeginIndexPortId3;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (nLength / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	default:
		beginIndex = 0;
		endIndex = 0;
		bUpdate = false;
		break;
	}

#ifndef NDEBUG
#if defined (__circle__)
	CLogger::Get ()->Write(__FUNCTION__, LogDebug, "%u %u %u %s", nPortId, beginIndex, endIndex, bUpdate == false ? "False" : "True");
#else
	monitor_line(MONITOR_LINE_STATS, "%d-%d:%x %x %x-%d|%s", nPortId, m_nDmxStartAddress, pData[0], pData[1], pData[2], nLength, bUpdate == false ? "False" : "True");
#endif
#endif

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	for (uint16_t j = beginIndex; j < endIndex; j++) {
		if (m_LEDType == SK6812W) {
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

	if (bUpdate) {
		m_pLEDStripe->Update();
		m_bIsStarted = true;
	}
}

void SPISend::SetLEDType(TWS28XXType type) {
	m_LEDType = type;

	if (type == SK6812W) {
		m_nBeginIndexPortId1 = 128;
		m_nBeginIndexPortId2 = 256;
		m_nBeginIndexPortId3 = 384;

		m_nChannelsPerLed = 4;
	}

	UpdateMembers();
}

TWS28XXType SPISend::GetLEDType(void) const {
	return m_LEDType;
}

void SPISend::SetLEDCount(uint16_t nCount) {
	m_nLEDCount = nCount;

	UpdateMembers();
}

uint16_t SPISend::GetLEDCount(void) const {
	return m_nLEDCount;
}

bool SPISend::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS));

	//FIXME Footprint

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS)) {
		m_nDmxStartAddress = nDmxStartAddress;
		return true;
	}

	return false;
}

bool SPISend::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	unsigned nIndex;

	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	if (m_LEDType == SK6812W) {
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

void SPISend::UpdateMembers(void) {
	m_nDmxFootprint = m_nLEDCount * m_nChannelsPerLed;

	if (m_nDmxFootprint > DMX_MAX_CHANNELS) {
		m_nDmxFootprint = DMX_MAX_CHANNELS;
	}
}
