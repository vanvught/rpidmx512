/**
 * @file tlc59711dmx.cpp
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
#include <cassert>

#include "tlc59711dmx.h"
#include "tlc59711.h"

#include "lightset.h"
#include "lightsetdisplay.h"

static unsigned long ceil(float f) {
	int i = static_cast<int>(f);

	if (f == static_cast<float>(i)) {
		return static_cast<unsigned long>(i);
	}

	return static_cast<unsigned long>(i + 1);
}

TLC59711Dmx::TLC59711Dmx() : m_nDmxFootprint(TLC59711Channels::OUT), m_nLEDCount(TLC59711Channels::RGB) {
	UpdateMembers();
}

TLC59711Dmx::~TLC59711Dmx() {
	delete m_pTLC59711;
	m_pTLC59711 = nullptr;
}

void TLC59711Dmx::Start(__attribute__((unused)) uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (__builtin_expect((m_pTLC59711 == nullptr), 0)) {
		Initialize();
	}
}

void TLC59711Dmx::Stop(__attribute__((unused)) uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void TLC59711Dmx::SetData(__attribute__((unused)) uint8_t nPort, const uint8_t* pDmxData, uint16_t nLength) {
	assert(pDmxData != nullptr);
	assert(nLength <= DMX_UNIVERSE_SIZE);

	if (__builtin_expect((m_pTLC59711 == nullptr), 0)) {
		Start();
	}

	uint8_t *p = const_cast<uint8_t*>(pDmxData) + m_nDmxStartAddress - 1;

	unsigned nDmxAddress = m_nDmxStartAddress;

	for (unsigned i = 0; i < m_nDmxFootprint; i++) {
		if (nDmxAddress > nLength) {
			break;
		}

		const uint16_t nValue = (static_cast<uint16_t>(*p) << 8) | static_cast<uint16_t>(*p);

		m_pTLC59711->Set(i, nValue);

		p++;
		nDmxAddress++;
	}

	if (__builtin_expect((nDmxAddress == m_nDmxStartAddress), 0)) {
		return;
	}

	if (!m_bBlackout) {
		m_pTLC59711->Update();
	}
}

void TLC59711Dmx::SetLEDType(TTLC59711Type tTLC59711Type) {
	m_LEDType = tTLC59711Type;
	UpdateMembers();
}

void TLC59711Dmx::SetLEDCount(uint8_t nLEDCount) {
	m_nLEDCount = nLEDCount;
	UpdateMembers();
}

void TLC59711Dmx::SetSpiSpeedHz(uint32_t nSpiSpeedHz) {
	m_nSpiSpeedHz = nSpiSpeedHz;
}

void TLC59711Dmx::Initialize() {
	assert(m_pTLC59711 == nullptr);
	m_pTLC59711 = new TLC59711(m_nBoardInstances, m_nSpiSpeedHz);
	assert(m_pTLC59711 != nullptr);
	m_pTLC59711->Dump();
}

void TLC59711Dmx::UpdateMembers() {
	if (m_LEDType == TTLC59711_TYPE_RGB) {
		m_nDmxFootprint = m_nLEDCount * 3;
	} else {
		m_nDmxFootprint = m_nLEDCount * 4;
	}

	m_nBoardInstances = ceil(static_cast<float>(m_nDmxFootprint) / TLC59711Channels::OUT);
}

void TLC59711Dmx::Blackout(bool bBlackout) {
	m_bBlackout = bBlackout;

	if (bBlackout) {
		m_pTLC59711->Blackout();
	} else {
		m_pTLC59711->Update();
	}
}

// DMX

#define BOARD_INSTANCES_MAX	32

bool TLC59711Dmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_UNIVERSE_SIZE));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_UNIVERSE_SIZE)) {
		m_nDmxStartAddress = nDmxStartAddress;

		if (m_pTLC59711DmxStore != nullptr) {
			m_pTLC59711DmxStore->SaveDmxStartAddress(m_nDmxStartAddress);
		}

		if (m_pLightSetDisplay != nullptr) {
			m_pLightSetDisplay->ShowDmxStartAddress();
		}

		return true;
	}

	return false;
}

// RDM

#define MOD(a,b)	(static_cast<unsigned>(a - b) * (static_cast<unsigned>(a/b)))

bool TLC59711Dmx::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	unsigned nIndex;

	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	if (m_LEDType == TTLC59711_TYPE_RGB) {
		nIndex = MOD(nSlotOffset, 3);
	} else {
		nIndex = MOD(nSlotOffset, 4);
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
