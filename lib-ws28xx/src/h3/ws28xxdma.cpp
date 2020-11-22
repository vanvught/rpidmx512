/**
 * @file ws28xxdma.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "h3/ws28xxdma.h"
#include "ws28xx.h"

#include "h3_spi.h"

#include "debug.h"

WS28xxDMA::WS28xxDMA(TWS28XXType Type, uint16_t nLEDCount, TRGBMapping tRGBMapping, uint8_t nT0H, uint8_t nT1H, uint32_t nClockSpeed):
	WS28xx(Type, nLEDCount, tRGBMapping, nT0H, nT1H, nClockSpeed)
{
	DEBUG_ENTRY

	DEBUG_EXIT
}

WS28xxDMA::~WS28xxDMA() {
	m_pBlackoutBuffer = nullptr;
	m_pBuffer = nullptr;
}

bool WS28xxDMA::Initialize() {
	uint32_t nSize;

	m_pBuffer = const_cast<uint8_t*>(h3_spi_dma_tx_prepare(&nSize));
	assert(m_pBuffer != nullptr);

	const uint32_t nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	if (m_nBufSize > nSizeHalf) {
		return false;
	}

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));

	if (m_tLEDType == APA102) {
		memset(m_pBuffer, 0, 4);
		for (uint32_t i = 0; i < m_nLedCount; i++) {
			SetLED(i, 0, 0, 0);
		}
		memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
	} else {
		memset(m_pBuffer, m_tLEDType == WS2801 ? 0 : 0xC0, m_nBufSize);
	}

	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

	DEBUG_PRINTF("nSize=%x, m_pBuffer=%p, m_pBlackoutBuffer=%p", nSize, m_pBuffer, m_pBlackoutBuffer);

	Blackout();

	return true;
}

void WS28xxDMA::Update() {
	assert(m_pBuffer != nullptr);
	assert(!IsUpdating());

	h3_spi_dma_tx_start(m_pBuffer, m_nBufSize);
}

void WS28xxDMA::Blackout() {
	assert(m_pBlackoutBuffer != nullptr);
	assert(!IsUpdating());

	h3_spi_dma_tx_start(m_pBlackoutBuffer, m_nBufSize);
}
