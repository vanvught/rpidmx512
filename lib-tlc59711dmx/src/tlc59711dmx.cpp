/**
 * @file tlc59711dmx.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "tlc59711dmx.h"
#include "tlc59711dmxstore.h"
#include "tlc59711.h"

#include "lightset.h"

#include "debug.h"

TLC59711Dmx *TLC59711Dmx::s_pThis;

TLC59711Dmx::TLC59711Dmx() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	UpdateMembers();

	DEBUG_EXIT
}

TLC59711Dmx::~TLC59711Dmx() {
	DEBUG_ENTRY

	if (m_pTLC59711 != nullptr) {
		delete m_pTLC59711;
		m_pTLC59711 = nullptr;
	}
#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
	if (m_ArrayMaxValue != nullptr) {
		delete[] m_ArrayMaxValue;
		m_ArrayMaxValue = nullptr;
	}
#endif

	DEBUG_EXIT
}

void TLC59711Dmx::Start([[maybe_unused]] uint32_t nPortIndex) {
	if (m_bIsStarted) {
		assert(m_pTLC59711 != nullptr);
		return;
	}

	m_bIsStarted = true;

	if (m_pTLC59711 == nullptr) {
		Initialize();
	}
}

void TLC59711Dmx::Stop([[maybe_unused]] uint32_t nPortIndex) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void TLC59711Dmx::Initialize() {
	auto nBoardInstances = static_cast<uint8_t>((m_nDmxFootprint + (TLC59711Channels::OUT - 1)) / TLC59711Channels::OUT);

	assert(m_pTLC59711 == nullptr);
	m_pTLC59711 = new TLC59711(nBoardInstances, m_nSpiSpeedHz);
	assert(m_pTLC59711 != nullptr);

#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
	assert(m_ArrayMaxValue == nullptr);
	m_ArrayMaxValue = new uint16_t[m_nDmxFootprint];
	assert(m_ArrayMaxValue != nullptr);

	for (uint32_t nIndex = 0; nIndex < m_nDmxFootprint; nIndex++) {
		m_ArrayMaxValue[nIndex] = UINT16_MAX;
	}
#endif

	m_pTLC59711->Dump();
}

#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
void TLC59711Dmx::SetMaxPct(uint32_t nIndexLed, uint32_t nPct) {
	assert(nIndexLed < m_nCount);

	if (nPct > 100) {
		nPct = 100;
	}

	const auto nMaxValue = static_cast<uint16_t>((UINT16_MAX * nPct) / 100U);

	if (m_type == tlc59711::Type::RGB) {
		const auto nIndexArray = nIndexLed * 3U;
		for (uint32_t i = 0; i < 3; i++) {
			m_ArrayMaxValue[i + nIndexArray] = nMaxValue;
		}
	} else {
		const auto nIndexArray = nIndexLed * 4U;
		for (uint32_t i = 0; i < 4; i++) {
			m_ArrayMaxValue[i + nIndexArray] = nMaxValue;
		}
	}

# if 0
	for (uint32_t nIndexLed = 0; nIndexLed < m_nCount; nIndexLed++) {
		if (m_type == tlc59711::Type::RGB) {
			const auto nIndexArray = nIndexLed * 3U;
			for (uint32_t i = 0; i < 3; i++) {
				printf("%.4x ", m_ArrayMaxValue[i + nIndexArray]);
			}
			puts("");
		} else {
			const auto nIndexArray = nIndexLed * 4U;
			for (uint32_t i = 0; i < 4; i++) {
				printf("%.4x ", m_ArrayMaxValue[i + nIndexArray]);
			}
			puts("");
		}
	}
# endif
}
#endif

void TLC59711Dmx::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pDmxData, uint32_t nLength, const bool doUpdate) {
	assert(pDmxData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	if (__builtin_expect((m_pTLC59711 == nullptr), 0)) {
		Start();
	}

	auto *p = const_cast<uint8_t *>(pDmxData) + m_nDmxStartAddress - 1;
	auto nDmxAddress = m_nDmxStartAddress;

	for (uint32_t i = 0; i < m_nDmxFootprint; i++) {
		if (nDmxAddress > nLength) {
			break;
		}

		auto nValue = static_cast<uint16_t>((*p << 8) | *p);

#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
		if (nValue > m_ArrayMaxValue[i]) {
			nValue = m_ArrayMaxValue[i];
		}
#endif
		m_pTLC59711->Set(i, nValue);// NOLINT(clang-analyzer-core.CallAndMessage): Start() ensures m_pTLC59711 is not nullptr

		p++;
		nDmxAddress++;
	}

	if (__builtin_expect((nDmxAddress == m_nDmxStartAddress), 0)) {
		return;
	}

	if ((doUpdate) && (!m_bBlackout)) {
		m_pTLC59711->Update();
	}
}

void TLC59711Dmx::Sync([[maybe_unused]] uint32_t const nPortIndex) {
	// No actions here
}

void TLC59711Dmx::Sync() {
	if (!m_bBlackout) {
		m_pTLC59711->Update();
	}
}

void TLC59711Dmx::SetType(tlc59711::Type type) {
	m_type = type;
	UpdateMembers();
}

void TLC59711Dmx::SetCount(uint32_t nCount) {
	m_nCount = static_cast<uint16_t>(nCount);
	UpdateMembers();
}

void TLC59711Dmx::SetSpiSpeedHz(uint32_t nSpiSpeedHz) {
	m_nSpiSpeedHz = nSpiSpeedHz;
}

void TLC59711Dmx::UpdateMembers() {
	if (m_type == tlc59711::Type::RGB) {
		m_nDmxFootprint = m_nCount * 3U;
	} else {
		m_nDmxFootprint = m_nCount * 4U;
	}
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

bool TLC59711Dmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= (lightset::dmx::UNIVERSE_SIZE - m_nDmxFootprint)));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= (lightset::dmx::UNIVERSE_SIZE - m_nDmxFootprint))) {
		m_nDmxStartAddress = nDmxStartAddress;
		TLC59711DmxStore::SaveDmxStartAddress(m_nDmxStartAddress);
		return true;
	}

	return false;
}

// RDM

bool TLC59711Dmx::GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo& slotInfo) {
	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	uint32_t nIndex;

	if (m_type == tlc59711::Type::RGB) {
		nIndex = nSlotOffset % 3U;
	} else {
		nIndex = nSlotOffset % 4U;
	}

	slotInfo.nType = 0x00;	// ST_PRIMARY

	switch (nIndex) {
		case 0:
			slotInfo.nCategory = 0x0205; // SD_COLOR_ADD_RED
			break;
		case 1:
			slotInfo.nCategory = 0x0206; // SD_COLOR_ADD_GREEN
			break;
		case 2:
			slotInfo.nCategory = 0x0207; // SD_COLOR_ADD_BLUE
			break;
		case 3:
			slotInfo.nCategory = 0x0212; // SD_COLOR_ADD_WHITE
			break;
		default:
			assert(0);
			break;
	}

	return true;
}
