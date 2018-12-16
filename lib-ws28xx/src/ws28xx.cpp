/**
 * @file ws28xx.cpp
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
#include <string.h>
#include <assert.h>

#if defined(__linux__)
 #include "bcm2835.h"
#elif defined(H3)
 #include "h3_spi.h"
#else
 #include "bcm2835_spi.h"
#endif

#include "ws28xx.h"

WS28xx::WS28xx(TWS28XXType Type, uint16_t nLEDCount, uint32_t nClockSpeed) :
	m_tLEDType(Type),
	m_nLEDCount(nLEDCount),
	m_nClockSpeedHz(nClockSpeed),
	m_nGlobalBrightness(0xFF),
	m_bUpdating(false),
	m_nHighCode(Type == WS2812B ? 0xF8 : 0xF0)
{
	assert(m_tLEDType <= APA102);
	assert(m_nLEDCount > 0);

	if ((m_tLEDType == SK6812W) || (m_tLEDType == APA102)) {
		m_nBufSize = nLEDCount * 4;
	} else {
		m_nBufSize = nLEDCount * 3;
	}

	if (m_tLEDType == WS2811 || m_tLEDType == WS2812 || m_tLEDType == WS2812B || m_tLEDType == WS2813 || m_tLEDType == WS2815 || m_tLEDType == SK6812 || m_tLEDType == SK6812W) {
		m_nBufSize *= 8;
	}

	if (m_tLEDType == APA102) {
		m_nBufSize += 8;
	}

	m_pBuffer = new uint8_t[m_nBufSize];
	assert(m_pBuffer != 0);
	if (m_tLEDType == APA102) {
		memset(m_pBuffer, 0, 4);
		for (uint32_t i = 0; i < m_nLEDCount; i++) {
			SetLED(i, 0, 0, 0);
		}
		memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
	} else {
		memset(m_pBuffer, m_tLEDType == WS2801 ? 0 : 0xC0, m_nBufSize);
	}

	m_pBlackoutBuffer = new uint8_t[m_nBufSize];
	assert(m_pBlackoutBuffer != 0);
	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

#ifdef H3
	if (!((m_tLEDType == WS2801) || (m_tLEDType == APA102))) {
		h3_spi_set_ws28xx_mode(true);
	}
#endif

	bcm2835_spi_begin();

	if ((Type == WS2801) || (Type == APA102)){
		if (nClockSpeed == 0) {
			m_nClockSpeedHz = WS2801_SPI_SPEED_DEFAULT_HZ;
			bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) WS2801_SPI_SPEED_DEFAULT_HZ));
		} else if (nClockSpeed < WS2801_SPI_SPEED_MAX_HZ) {
			bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / nClockSpeed));
		} else {
			m_nClockSpeedHz = WS2801_SPI_SPEED_MAX_HZ;
			bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) WS2801_SPI_SPEED_MAX_HZ));
		}
	} else {
		bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) 6400000));
	}

	Update();
}

WS28xx::~WS28xx(void) {
	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;
}

void WS28xx::Update(void) {
	assert (m_pBuffer != 0);

	__sync_synchronize();
	bcm2835_spi_writenb((char *) m_pBuffer, m_nBufSize);
}

void WS28xx::Blackout(void) {
	assert (m_pBlackoutBuffer != 0);

	__sync_synchronize();
	bcm2835_spi_writenb((char *) m_pBlackoutBuffer, m_nBufSize);
}
