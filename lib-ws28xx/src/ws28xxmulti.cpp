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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ws28xxmulti.h"

#include "pixelconfiguration.h"
#include "pixeltype.h"

#include "debug.h"

using namespace ws28xxmulti;
using namespace pixel;

WS28xxMulti *WS28xxMulti::s_pThis = nullptr;

WS28xxMulti::WS28xxMulti(PixelConfiguration& pixelConfiguration) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	uint32_t nLedsPerPixel;
	pixelConfiguration.Validate(nLedsPerPixel);
	pixelConfiguration.Dump();

	m_Type = pixelConfiguration.GetType();
	m_nCount = pixelConfiguration.GetCount();
	m_Map = pixelConfiguration.GetMap();
	m_bIsRTZProtocol = pixelConfiguration.IsRTZProtocol();
	m_nGlobalBrightness = pixelConfiguration.GetGlobalBrightness();
	m_nBufSize = m_nCount * nLedsPerPixel;

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		m_nBufSize += m_nCount;
		m_nBufSize += 8;
	}

	m_nBufSize *= 8;

	DEBUG_PRINTF("m_nBufSize=%d", m_nBufSize);

	m_Board = GetBoard();

	DEBUG_PRINTF("m_Board=%d [%dx]", static_cast<int>(m_Board), m_Board == Board::X4 ? 4 : 8);

	const auto nLowCode = pixelConfiguration.GetLowCode();
	const auto nHighCode = pixelConfiguration.GetHighCode();

	if (m_Board == Board::X4) {
		SetupMCP23017(ReverseBits(nLowCode), ReverseBits(nHighCode));
		SetupSI5351A();
		SetupGPIO();
		SetupBuffers4x();
	} else {
		m_hasCPLD = SetupCPLD();
		SetupHC595(ReverseBits(nLowCode), ReverseBits(nHighCode));
		if (pixelConfiguration.IsRTZProtocol()) {
			SetupSPI(pixelConfiguration.GetClockSpeedHz());
		} else {
			if(m_hasCPLD) {
				SetupSPI(pixelConfiguration.GetClockSpeedHz() * 6);
			} else {
				SetupSPI(pixelConfiguration.GetClockSpeedHz() * 4);
			}
		}
		m_nBufSize++;
		SetupBuffers8x();
	}

	DEBUG_EXIT
}

WS28xxMulti::~WS28xxMulti() {
	if (m_Board == Board::X4) {
		delete[] m_pBlackoutBuffer4x;
		m_pBlackoutBuffer4x = nullptr;

		delete[] m_pBuffer4x;
		m_pBuffer4x = nullptr;
	} else {
		m_pBlackoutBuffer8x = nullptr;
		m_pBuffer8x = nullptr;
	}

	s_pThis = nullptr;
}

void WS28xxMulti::Print() {
	printf("Pixel parameters\n");
	printf(" Type    : %s [%d] - %s [%d]\n", PixelType::GetType(m_Type), static_cast<int>(m_Type), PixelType::GetMap(m_Map), static_cast<int>(m_Map));
	printf(" Count   : %d\n", m_nCount);
	printf(" Board   : %dx (%s)\n", m_Board == ws28xxmulti::Board::X4 ? 4 : 8, m_hasCPLD ? "CPLD" : "74-logic");
}

ws28xxmulti::Board WS28xxMulti::GetBoard() {
	return (IsMCP23017() ? Board::X4 : Board::X8);
}
