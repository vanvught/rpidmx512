/**
 * @file ws28xxstripe.cpp
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

#include <stdint.h>
#include <assert.h>

#include "bcm2835_spi.h"
#include "util.h"

#include "ws28xxstripe.h"

WS28XXStripe::WS28XXStripe(const uint16_t nLEDCount, const TWS28XXType Type, const uint32_t nClockSpeed) {
	m_nLEDCount = nLEDCount;
	m_Type = Type;
	m_nHighCode = Type == WS2812B ? 0xF8 : 0xF0;

	if (Type == SK6812W) {
		m_nBufSize = nLEDCount * 4;
	} else {
		m_nBufSize = nLEDCount * 3;
	}

	if (Type == WS2811 || Type == WS2812 || Type == WS2812B || Type == WS2813 || Type == SK6812 || Type == SK6812W) {
		m_nBufSize *= 8;
	}

	m_pBuffer = new uint8_t[m_nBufSize];
	assert(m_pBuffer != 0);
	memset(m_pBuffer, m_Type == WS2801 ? 0 : 0xC0, m_nBufSize);

	m_pBlackoutBuffer = new uint8_t[m_nBufSize];
	assert(m_pBlackoutBuffer != 0);
	memset(m_pBlackoutBuffer, m_Type == WS2801 ? 0 : 0xC0, m_nBufSize);

	bcm2835_spi_begin();

	if (Type == WS2801) {
		if (nClockSpeed == (uint32_t) 0) {
			bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) WS2801_SPI_SPEED_DEFAULT_HZ));
		} else {
			bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / nClockSpeed));
		}
	} else {
		bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) 6400000));
	}

	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

	Update();
}

WS28XXStripe::~WS28XXStripe(void) {
	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;
}

unsigned WS28XXStripe::GetLEDCount(void) const {
	return m_nLEDCount;
}

TWS28XXType WS28XXStripe::GetLEDType(void) const {
	return m_Type;
}

void WS28XXStripe::SetLED(unsigned nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
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

void WS28XXStripe::Update(void) {
	assert (m_pBuffer != 0);
	__sync_synchronize();
	bcm2835_spi_writenb((char *) m_pBuffer, m_nBufSize);
}

void WS28XXStripe::Blackout(void) {
	assert (m_pBlackoutBuffer != 0);
	__sync_synchronize();
	bcm2835_spi_writenb((char *) m_pBlackoutBuffer, m_nBufSize);
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
