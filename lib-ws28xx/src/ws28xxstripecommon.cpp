/**
 * @file ws28xxstripecommon.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>

#include "ws28xxstripe.h"

void WS28XXStripe::SetLED(unsigned nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(!m_bUpdating);

	assert(m_pBuffer != 0);
	assert(nLEDIndex < m_nLEDCount);
	unsigned nOffset = nLEDIndex * 3;

	if (m_Type == WS2801) {
		assert(nOffset + 2 < m_nBufSize);
		m_pBuffer[nOffset] = nRed;
		m_pBuffer[nOffset + 1] = nGreen;
		m_pBuffer[nOffset + 2] = nBlue;
	} else if (m_Type == WS2811) {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nRed);
		SetColorWS28xx(nOffset + 8, nGreen);
		SetColorWS28xx(nOffset + 16, nBlue);
	} else {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
	}
}

void WS28XXStripe::SetLED(unsigned nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(!m_bUpdating);

	assert(m_pBuffer != 0);
	assert(nLEDIndex < m_nLEDCount);
	assert(m_Type == SK6812W);

	unsigned nOffset = nLEDIndex * 4;

	if (m_Type == SK6812W) {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
		SetColorWS28xx(nOffset + 24, nWhite);
	}
}

void WS28XXStripe::SetColorWS28xx(unsigned nOffset, uint8_t nValue) {
	assert(m_Type != WS2801);
	uint8_t mask;

	assert(nOffset + 7 < m_nBufSize);

	for (mask = 0x80; mask != 0; mask >>= 1) {
		if (nValue & mask) {
			m_pBuffer[nOffset] = m_nHighCode;
		} else {
			m_pBuffer[nOffset] = 0xC0;	// Same for all
		}

		nOffset++;
	}
}

unsigned WS28XXStripe::GetLEDCount(void) const {
	return m_nLEDCount;
}

TWS28XXType WS28XXStripe::GetLEDType(void) const {
	return m_Type;
}
