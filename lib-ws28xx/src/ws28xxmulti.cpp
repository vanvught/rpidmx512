/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "ws28xxmulti.h"

#include "debug.h"

using namespace ws28xxmulti;
using namespace ws28xx;

static Type s_NotSupported[] = {Type::WS2801, Type::APA102, Type::P9813};	// SPI Clock based

WS28xxMulti *WS28xxMulti::s_pThis = nullptr;

WS28xxMulti::WS28xxMulti() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_tBoard = (IsMCP23017() ? Board::X4 : Board::X8);

	DEBUG_PRINTF("m_tBoard=%d [%dx]", static_cast<int>(m_tBoard), m_tBoard == Board::X4 ? 4 : 8);
	DEBUG_EXIT
}

WS28xxMulti::~WS28xxMulti() {
	if (m_tBoard == Board::X4) {
		delete[] m_pBlackoutBuffer4x;
		m_pBlackoutBuffer4x = nullptr;

		delete[] m_pBuffer4x;
		m_pBuffer4x = nullptr;
	} else {
		m_pBlackoutBuffer8x = nullptr;
		m_pBuffer8x = nullptr;
	}
}

void WS28xxMulti::Initialize(Type tWS28xxType, uint16_t nLedCount, __attribute__((unused)) rgbmapping::Map tRGBMapping, __attribute__((unused)) uint8_t nT0H, __attribute__((unused)) uint8_t nT1H, bool bUseSI5351A) {
	DEBUG_ENTRY
	assert(nLedCount > 0);

	m_tWS28xxType = tWS28xxType;

	for (uint32_t i = 0; i < sizeof(s_NotSupported) / sizeof(s_NotSupported[0]) ; i++) {
		if (tWS28xxType == s_NotSupported[i]) {
			m_tWS28xxType = Type::WS2812B;
			break;
		}
	}

	DEBUG_PRINTF("m_tWS28xxType=%d (%s), m_nLedCount=%d, m_nBufSize=%d", static_cast<int>(m_tWS28xxType), WS28xx::GetLedTypeString(m_tWS28xxType), m_nLedCount, m_nBufSize);
	DEBUG_PRINTF("m_tRGBMapping=%d (%s), m_nLowCode=0x%X, m_nHighCode=0x%X", m_tRGBMapping	, RGBMapping::ToString(m_tRGBMapping), m_nLowCode, m_nHighCode);

	if (m_tRGBMapping == rgbmapping::Map::UNDEFINED) {
		m_tRGBMapping = WS28xx::GetRgbMapping(m_tWS28xxType);
	}

	uint8_t nLowCode, nHighCode;

	WS28xx::GetTxH(m_tWS28xxType, nLowCode, nHighCode);

	if (m_nLowCode == 0) {
		m_nLowCode = nLowCode;
	}

	if (m_nHighCode == 0) {
		m_nHighCode = nHighCode;
	}

	if (m_tWS28xxType == Type::SK6812W) {
		m_nLedCount = nLedCount <= static_cast<uint16_t>(ws28xx::max::ledcount::RGBW) ? nLedCount : static_cast<uint16_t>(ws28xx::max::ledcount::RGBW);
		m_nBufSize = static_cast<uint32_t>(nLedCount * ws28xx::single::RGBW);
	} else {
		m_nLedCount = nLedCount <= static_cast<uint16_t>(ws28xx::max::ledcount::RGB) ? nLedCount : static_cast<uint16_t>(ws28xx::max::ledcount::RGB);
		m_nBufSize = static_cast<uint32_t>(nLedCount * ws28xx::single::RGB);
	}

	DEBUG_PRINTF("m_tWS28xxType=%d (%s), m_nLedCount=%d, m_nBufSize=%d", static_cast<int>(m_tWS28xxType), WS28xx::GetLedTypeString(m_tWS28xxType), m_nLedCount, m_nBufSize);
	DEBUG_PRINTF("m_tRGBMapping=%d (%s), m_nLowCode=0x%X, m_nHighCode=0x%X", m_tRGBMapping, RGBMapping::ToString(m_tRGBMapping), m_nLowCode, m_nHighCode);

	if (m_tBoard == Board::X4) {
		SetupMCP23017(ReverseBits(m_nLowCode), ReverseBits(m_nHighCode));
		if (bUseSI5351A) {
			SetupSI5351A();
		}
		SetupGPIO();
		SetupBuffers4x();
	} else {
		SetupHC595(ReverseBits(m_nLowCode), ReverseBits(m_nHighCode));
		SetupSPI();
		m_nBufSize++;
		SetupBuffers8x();
	}

	DEBUG_PRINTF("m_nLedCount=%d, m_nBufSize=%d", m_nLedCount,m_nBufSize);
	DEBUG_EXIT
}
