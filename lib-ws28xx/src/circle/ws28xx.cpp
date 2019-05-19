/**
 * @file ws28xxstripe.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Based on https://github.com/rsta2/circle/tree/master/addon/WS28XX
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

#include <string.h>
#include <assert.h>

#include <circle/logger.h>

#include "ws28xx.h"

WS28xx::WS28xx (CInterruptSystem *pInterruptSystem, TWS28XXType Type, uint16_t nLEDCount, uint32_t nClockSpeed)
:	m_tLEDType (Type),
	m_nLEDCount (nLEDCount),
	m_nClockSpeedHz(nClockSpeed),
	m_nGlobalBrightness(0xFF),
	m_bUpdating (FALSE),
	m_nHighCode(Type == WS2812B ? 0xF8 : (((Type == UCS1903) || (Type == UCS2903)) ? 0xFC : 0xF0)),
	m_SPIMaster (pInterruptSystem, ((m_tLEDType == WS2801) || (m_tLEDType == APA102)) ? (nClockSpeed == 0 ? WS2801_SPI_SPEED_DEFAULT_HZ : nClockSpeed) : 6400000, 0, 0)
{
	assert(m_tLEDType <= UCS2903);
	assert(m_nLEDCount > 0);

	if ((m_tLEDType == SK6812W) || (m_tLEDType == APA102)) {
		m_nBufSize = m_nLEDCount * 4;
	} else {
		m_nBufSize = m_nLEDCount * 3;
	}

	if (m_tLEDType == WS2811 || m_tLEDType == WS2812 || m_tLEDType == WS2812B || m_tLEDType == WS2813 || m_tLEDType == WS2815 || m_tLEDType == SK6812 || m_tLEDType == SK6812W || m_tLEDType == UCS1903 || m_tLEDType == UCS2903) {
		m_nBufSize *= 8;
	}

	if (m_tLEDType == APA102) {
		m_nBufSize += 8;
	}

	m_pBuffer = new u8[m_nBufSize];
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

	m_pReadBuffer = new u8[m_nBufSize];
	assert(m_pReadBuffer != 0);

	m_pBlackoutBuffer = new u8[m_nBufSize];
	assert(m_pBlackoutBuffer != 0);
	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);
}

WS28xx::~WS28xx(void) {
	while (m_bUpdating) {
		// just wait
	}

	delete[] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete[] m_pReadBuffer;
	m_pReadBuffer = 0;

	delete[] m_pBuffer;
	m_pBuffer = 0;
}

bool WS28xx::Initialize(void) {
	return m_SPIMaster.Initialize();
}

void WS28xx::Update(void) {
	assert(!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine(SPICompletionStub, this);

	assert(m_pBuffer != 0);
	assert(m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead(0, m_pBuffer, m_pReadBuffer, m_nBufSize);
}

void WS28xx::Blackout(void) {
	assert(!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine(SPICompletionStub, this);

	assert(m_pBlackoutBuffer != 0);
	assert(m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead(0, m_pBlackoutBuffer, m_pReadBuffer, m_nBufSize);
}

bool WS28xx::IsUpdating(void) const {
	return m_bUpdating;
}

void WS28xx::SPICompletionRoutine(boolean bStatus) {
	if (!bStatus) {
		CLogger::Get()->Write(__FUNCTION__, LogError, "SPI DMA operation failed");
	}

	assert(m_bUpdating);
	m_bUpdating = FALSE;
}

void WS28xx::SPICompletionStub(boolean bStatus, void *pParam) {
	WS28xx *pThis = (WS28xx *) pParam;
	assert(pThis != 0);

	pThis->SPICompletionRoutine(bStatus);
}

