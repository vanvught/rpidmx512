/**
 * @file ws28xxset.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "ws28xx.h"

using namespace pixel;

void WS28xx::SetPixel(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(m_pBuffer != nullptr);
	assert(nLEDIndex < m_nCount);

	if (__builtin_expect((m_bIsRTZProtocol), 1)) {
		uint32_t nOffset = nLEDIndex * 3;
		nOffset *= 8;

		switch (m_Map) {
		case Map::RGB:
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		case Map::RBG:
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nBlue);
			SetColorWS28xx(nOffset + 16, nGreen);
			break;
		case Map::GRB:
			SetColorWS28xx(nOffset, nGreen);
			SetColorWS28xx(nOffset + 8, nRed);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		case Map::GBR:
			SetColorWS28xx(nOffset, nGreen);
			SetColorWS28xx(nOffset + 8, nBlue);
			SetColorWS28xx(nOffset + 16, nRed);
			break;
		case Map::BRG:
			SetColorWS28xx(nOffset, nBlue);
			SetColorWS28xx(nOffset + 8, nRed);
			SetColorWS28xx(nOffset + 16, nGreen);
			break;
		case Map::BGR:
			SetColorWS28xx(nOffset, nBlue);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nRed);
			break;
		default:  // RGB
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		}

		return;
	}

	if (m_Type == Type::APA102) {
		uint32_t nOffset = 4 + (nLEDIndex * 4);
		assert(nOffset + 3 < m_nBufSize);

		m_pBuffer[nOffset] = m_nGlobalBrightness;
		m_pBuffer[nOffset + 1] = nRed;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nBlue;

		return;
	}

	if (m_Type == Type::WS2801) {
		uint32_t nOffset = nLEDIndex * 3;
		assert(nOffset + 2 < m_nBufSize);

		m_pBuffer[nOffset] = nRed;
		m_pBuffer[nOffset + 1] = nGreen;
		m_pBuffer[nOffset + 2] = nBlue;

		return;
	}

	if (m_Type == Type::P9813) {
		uint32_t nOffset = 4 + (nLEDIndex * 4);
		assert(nOffset + 3 < m_nBufSize);

		const auto nFlag = static_cast<uint8_t>(0xC0 | ((~nBlue & 0xC0) >> 2) | ((~nGreen & 0xC0) >> 4) | ((~nRed & 0xC0) >> 6));

		m_pBuffer[nOffset] = nFlag;
		m_pBuffer[nOffset + 1] = nBlue;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nRed;

		return;
	}
}

void WS28xx::SetPixel(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(m_pBuffer != nullptr);
	assert(nLEDIndex < m_nCount);
	assert(m_Type == Type::SK6812W);

	uint32_t nOffset = nLEDIndex * 4;

	if (m_Type == Type::SK6812W) {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
		SetColorWS28xx(nOffset + 24, nWhite);
	}
}

void WS28xx::SetColorWS28xx(uint32_t nOffset, uint8_t nValue) {
	assert(m_Type != Type::WS2801);
	assert(nOffset + 7 < m_nBufSize);

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		if (nValue & mask) {
			m_pBuffer[nOffset] = m_nHighCode;
		} else {
			m_pBuffer[nOffset] = m_nLowCode;
		}
		nOffset++;
	}
}
