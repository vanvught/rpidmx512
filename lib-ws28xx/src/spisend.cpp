/**
 * @file spisend.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <assert.h>

#include "ws28xxstripe.h"

//#include "monitor.h"
#include "spisend.h"
#include "util.h"

SPISend::SPISend(void) : m_bIsStarted(false),
		m_pLEDStripe(0),
		m_LEDType(WS2801),
		m_nLEDCount(170),
		m_nBeginIndexPortId1(170),
		m_nBeginIndexPortId2(340),
		m_nBeginIndexPortId3(510),
		m_nChannelsPerLed(3) {
}

SPISend::~SPISend(void) {
	this->Stop();
	delete m_pLEDStripe;
	m_pLEDStripe = 0;
}

void SPISend::Start(void) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pLEDStripe == 0) {
		m_pLEDStripe = new WS28XXStripe(m_nLEDCount, m_LEDType, 0);
		assert(m_pLEDStripe != 0);
	} else {
		m_pLEDStripe->Update();
	}
}

void SPISend::Stop(void) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_pLEDStripe != 0) {
		m_pLEDStripe->Blackout();
	}
}

void SPISend::SetData(uint8_t nPortId, const uint8_t *data, uint16_t length) {
	uint16_t i = 0;
	uint16_t j = 0;

	uint16_t beginIndex = (uint16_t) 0;
	uint16_t endIndex = (uint16_t) 0;

	bool bUpdate = false;

	switch (nPortId) {
	case 0:
		beginIndex = (uint16_t) 0;
		endIndex = MIN(m_nLEDCount, (uint16_t) (length / (uint16_t) m_nChannelsPerLed));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 1:
		beginIndex = (uint16_t) m_nBeginIndexPortId1;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 2:
		beginIndex = (uint16_t) m_nBeginIndexPortId2;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 3:
		beginIndex = (uint16_t) m_nBeginIndexPortId3;
		endIndex = MIN(m_nLEDCount, (uint16_t) ((uint16_t) beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	default:
		break;
	}

	//monitor_line(MONITOR_LINE_STATS, "%d-%x:%x:%x-%d|%s", nPortId, data[0], data[1], data[2], length, bUpdate == false ? "False" : "True");

	if (__builtin_expect((m_pLEDStripe == 0), 0)) {
		Start();
	}

	if (m_LEDType == SK6812W) {
		for (j = beginIndex; j < endIndex; j++) {
			m_pLEDStripe->SetLED(j, data[i], data[i + 1], data[i + 2], data[i + 3]);
			i = i + 4;
		}
	} else {
		for (j = beginIndex; j < endIndex; j++) {
			m_pLEDStripe->SetLED(j, data[i], data[i + 1], data[i + 2]);
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
}

TWS28XXType SPISend::GetLEDType(void) const {
	return m_LEDType;
}

void SPISend::SetLEDCount(uint16_t nCount) {
	m_nLEDCount = nCount;
}

uint16_t SPISend::GetLEDCount(void) const {
	return m_nLEDCount;
}
