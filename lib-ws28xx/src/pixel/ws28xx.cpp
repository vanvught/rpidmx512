/**
 * @file ws28xx.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "ws28xx.h"
#include "pixeltype.h"

#include "gamma/gamma_tables.h"

using namespace pixel;

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("O3")
#endif

void WS28xx::SetPixel(uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPixelIndex < m_PixelConfiguration.GetCount());

	const auto pGammaTable = m_PixelConfiguration.GetGammaTable();

	nRed = pGammaTable[nRed];
	nGreen = pGammaTable[nGreen];
	nBlue = pGammaTable[nBlue];

	if (m_PixelConfiguration.IsRTZProtocol()) {
		const auto nOffset = nPixelIndex * 24U;

		switch (m_PixelConfiguration.GetMap()) {
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

	assert(m_pBuffer != nullptr);

	const auto type = m_PixelConfiguration.GetType();

	if ((type == Type::APA102) || (type == Type::SK9822)) {
		const auto nOffset = 4U + (nPixelIndex * 4U);
		assert(nOffset + 3U < m_nBufSize);

		m_pBuffer[nOffset] = m_PixelConfiguration.GetGlobalBrightness();
		m_pBuffer[nOffset + 1] = nRed;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nBlue;

		return;
	}

	if (type == Type::WS2801) {
		const auto nOffset = nPixelIndex * 3U;
		assert(nOffset + 2U < m_nBufSize);

		m_pBuffer[nOffset] = nRed;
		m_pBuffer[nOffset + 1] = nGreen;
		m_pBuffer[nOffset + 2] = nBlue;

		return;
	}

	if (type == Type::P9813) {
		const auto nOffset = 4U + (nPixelIndex * 4U);
		assert(nOffset + 3 < m_nBufSize);

		const auto nFlag = static_cast<uint8_t>(0xC0 | ((~nBlue & 0xC0) >> 2) | ((~nGreen & 0xC0) >> 4) | ((~nRed & 0xC0) >> 6));

		m_pBuffer[nOffset] = nFlag;
		m_pBuffer[nOffset + 1] = nBlue;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nRed;

		return;
	}

	assert(0);
	__builtin_unreachable();
}

void WS28xx::SetPixel(uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPixelIndex < m_PixelConfiguration.GetCount());
	assert(m_PixelConfiguration.GetType() == Type::SK6812W);

	const auto pGammaTable = m_PixelConfiguration.GetGammaTable();

	nRed = pGammaTable[nRed];
	nGreen = pGammaTable[nGreen];
	nBlue = pGammaTable[nBlue];
	nWhite = pGammaTable[nWhite];

	const auto nOffset = nPixelIndex * 32U;

	SetColorWS28xx(nOffset, nGreen);
	SetColorWS28xx(nOffset + 8, nRed);
	SetColorWS28xx(nOffset + 16, nBlue);
	SetColorWS28xx(nOffset + 24, nWhite);
}

void WS28xx::SetColorWS28xx(uint32_t nOffset, uint8_t nValue) {
	assert(m_PixelConfiguration.GetType() != Type::WS2801);
	assert(m_pBuffer != nullptr);
	assert(nOffset + 7 < m_nBufSize);

	nOffset += 1;

	const auto nLowCode = m_PixelConfiguration.GetLowCode();
	const auto nHighCode = m_PixelConfiguration.GetHighCode();

	for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
		if (nValue & mask) {
			m_pBuffer[nOffset] = nHighCode;
		} else {
			m_pBuffer[nOffset] = nLowCode;
		}
		nOffset++;
	}
}
