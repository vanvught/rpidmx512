/**
 * @file ws28xxcommon.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "ws28xx.h"

void WS28xx::SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(!m_bUpdating);

	assert(m_pBuffer != 0);
	assert(nLEDIndex < m_nLEDCount);

	if (m_tLEDType == APA102) {
		uint32_t nOffset = 4 + (nLEDIndex * 4);
		assert(nOffset + 3 < m_nBufSize);
		m_pBuffer[nOffset] = m_nGlobalBrightness;
		m_pBuffer[nOffset + 1] = nRed;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nBlue;
	} else if (m_tLEDType == WS2801) {
		uint32_t nOffset = nLEDIndex * 3;
		assert(nOffset + 2 < m_nBufSize);
		m_pBuffer[nOffset] = nRed;
		m_pBuffer[nOffset + 1] = nGreen;
		m_pBuffer[nOffset + 2] = nBlue;
	} else if ((m_tLEDType == WS2811) || (m_tLEDType == UCS2903)) {
		uint32_t nOffset = nLEDIndex * 3;
		nOffset *= 8;

		SetColorWS28xx(nOffset, nRed);
		SetColorWS28xx(nOffset + 8, nGreen);
		SetColorWS28xx(nOffset + 16, nBlue);
	} else if (m_tLEDType == UCS1903) {
		uint32_t nOffset = nLEDIndex * 3;
		nOffset *= 8;

		SetColorWS28xx(nOffset, nBlue);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nGreen);
	} else {
		uint32_t nOffset = nLEDIndex * 3;
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
	}
}

void WS28xx::SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(!m_bUpdating);

	assert(m_pBuffer != 0);
	assert(nLEDIndex < m_nLEDCount);
	assert(m_tLEDType == SK6812W);

	uint32_t nOffset = nLEDIndex * 4;

	if (m_tLEDType == SK6812W) {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
		SetColorWS28xx(nOffset + 24, nWhite);
	}
}

void WS28xx::SetColorWS28xx(uint32_t nOffset, uint8_t nValue) {
	assert(m_tLEDType != WS2801);
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

void WS28xx::SetGlobalBrightness(uint8_t nGlobalBrightness) {
	if (m_tLEDType == APA102) {
		if (nGlobalBrightness > 0x1F) {
			m_nGlobalBrightness = 0xFF;
		} else {
			m_nGlobalBrightness = 0xE0 | (nGlobalBrightness & 0x1F);
		}
	}
}
