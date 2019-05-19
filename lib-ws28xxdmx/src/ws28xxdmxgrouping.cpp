/**
 * @file ws28xxstripedmxgrouping.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ws28xxdmxgrouping.h"
#include "ws28xxdmxparams.h"

#include "lightset.h"

#define DMX_FOOTPRINT_DEFAULT		3

#if defined (__circle__)
WS28xxDmxGrouping::WS28xxDmxGrouping(CInterruptSystem *pInterruptSystem) : WS28xxDmx(pInterruptSystem) {
#else
WS28xxDmxGrouping::WS28xxDmxGrouping(void) {
#endif
	for (uint32_t i = 0; i < sizeof(m_aDmxData); i++) {
		m_aDmxData[i] = 0;
	}
}

WS28xxDmxGrouping::~WS28xxDmxGrouping(void) {
}

void WS28xxDmxGrouping::SetData(uint8_t nPort, const uint8_t* pData, uint16_t nLenght) {
	if (__builtin_expect((m_pLEDStripe == 0), 0)) {
		Start();
	}

	while (m_pLEDStripe->IsUpdating()) {
		// wait for completion
	}

	const uint8_t *p = pData + m_nDmxStartAddress - 1;
	bool bIsChanged = false;

	if (m_tLedType == SK6812W) {
		for (uint32_t i = 0; i < 4; i++) {
			if (p[i] != m_aDmxData[i]) {
				m_aDmxData[i] = p[i];
				bIsChanged = true;
			}
		}
		if (bIsChanged) {
			for (uint32_t j = 0; j < m_nLedCount; j++) {
				m_pLEDStripe->SetLED(j, m_aDmxData[0], m_aDmxData[1], m_aDmxData[2], m_aDmxData[3]);
			}
		}
	} else {
		for (uint32_t i = 0; i < 3; i++) {
			if (p[i] != m_aDmxData[i]) {
				m_aDmxData[i] = p[i];
				bIsChanged = true;
			}
		}
		if (bIsChanged) {
			for (uint32_t j = 0; j < m_nLedCount; j++) {
				m_pLEDStripe->SetLED(j, m_aDmxData[0], m_aDmxData[1], m_aDmxData[2]);
			}
		}
	}

	if (!m_bBlackout) {
		m_pLEDStripe->Update();
	}
}

void WS28xxDmxGrouping::SetLEDType(TWS28XXType tLedType) {
	m_tLedType = tLedType;

	if (tLedType == SK6812W) {
		m_nDmxFootprint = 4;
	}
}

void WS28xxDmxGrouping::SetLEDCount(uint16_t nLedCount) {
	m_nLedCount = nLedCount;
}

bool WS28xxDmxGrouping::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= (DMX_MAX_CHANNELS - m_nDmxFootprint)));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= (DMX_MAX_CHANNELS - m_nDmxFootprint))) {
		m_nDmxStartAddress = nDmxStartAddress;
		return true;
	}

	return false;
}

void WS28xxDmxGrouping::Print(void) {
	printf("Led (grouping) parameters\n");
	printf(" Type  : %s [%d]\n", WS28xxDmxParams::GetLedTypeString(m_tLedType), m_tLedType);
	printf(" Count : %d\n", (int) m_nLedCount);
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

