/**
 * @file ws28xxstripe.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#if defined(__linux__)
 #include "bcm2835.h"
 #include <string.h>
#elif defined(H3)
 #include "h3_spi.h"
 #include "util.h"
#else
 #include "bcm2835_spi.h"
 #include "util.h"
#endif

#include "ws28xxstripe.h"

WS28XXStripe::WS28XXStripe(TWS28XXType Type, uint16_t nLEDCount, uint32_t nClockSpeed) :
	m_Type(Type),
	m_nLEDCount(nLEDCount),
	m_bUpdating(false),
	m_nHighCode(Type == WS2812B ? 0xF8 : 0xF0)
{
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

#ifdef H3
	if (Type != WS2801) {
		h3_spi_set_ws28xx_mode(true);
	}
#endif

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

	Update();
}

WS28XXStripe::~WS28XXStripe(void) {
	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;
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
