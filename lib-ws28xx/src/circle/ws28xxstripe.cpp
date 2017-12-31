/**
 * @file ws28xxstripe.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Based on https://github.com/rsta2/circle/tree/master/addon/WS28XX
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

#include <circle/logger.h>
#include <circle/util.h>

#include "ws28xxstripe.h"

WS28XXStripe::WS28XXStripe (CInterruptSystem *pInterruptSystem, TWS28XXType Type, unsigned nLEDCount, unsigned nClockSpeed)
:	m_Type (Type),
	m_nLEDCount (nLEDCount),
	m_bUpdating (FALSE),
	m_nHighCode(Type == WS2812B ? 0xF8 : 0xF0),
	m_SPIMaster (pInterruptSystem, m_Type == WS2801 ? nClockSpeed : 6400000, 0, 0)
{
	assert(m_Type <= SK6812W);
	assert(m_nLEDCount > 0);

	if (m_Type == SK6812W) {
		m_nBufSize = m_nLEDCount * 4;
	} else {
		m_nBufSize = m_nLEDCount * 3;
	}

	if (m_Type == WS2811 || m_Type == WS2812 || m_Type == WS2812B || m_Type == WS2813 || m_Type == SK6812 || m_Type == SK6812W) {
		m_nBufSize *= 8;
	}

	m_pBuffer = new u8[m_nBufSize];
	assert(m_pBuffer != 0);

	for (unsigned nLEDIndex = 0; nLEDIndex < m_nLEDCount; nLEDIndex++) {
		SetLED(nLEDIndex, 0, 0, 0);
	}

	m_pReadBuffer = new u8[m_nBufSize];
	assert(m_pReadBuffer != 0);

	m_pBlackoutBuffer = new u8[m_nBufSize];
	assert(m_pBlackoutBuffer != 0);
	memset(m_pBlackoutBuffer, m_Type == WS2801 ? 0 : 0xC0, m_nBufSize);
}

WS28XXStripe::~WS28XXStripe(void) {
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

bool WS28XXStripe::Initialize(void) {
	return m_SPIMaster.Initialize();
}

void WS28XXStripe::Update(void) {
	assert(!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine(SPICompletionStub, this);

	assert(m_pBuffer != 0);
	assert(m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead(0, m_pBuffer, m_pReadBuffer, m_nBufSize);
}

void WS28XXStripe::Blackout(void) {
	assert(!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine(SPICompletionStub, this);

	assert(m_pBlackoutBuffer != 0);
	assert(m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead(0, m_pBlackoutBuffer, m_pReadBuffer, m_nBufSize);
}

bool WS28XXStripe::IsUpdating(void) const {
	return m_bUpdating;
}

void WS28XXStripe::SPICompletionRoutine(boolean bStatus) {
	if (!bStatus) {
		CLogger::Get()->Write(__FUNCTION__, LogError, "SPI DMA operation failed");
	}

	assert(m_bUpdating);
	m_bUpdating = FALSE;
}

void WS28XXStripe::SPICompletionStub(boolean bStatus, void *pParam) {
	WS28XXStripe *pThis = (WS28XXStripe *) pParam;
	assert(pThis != 0);

	pThis->SPICompletionRoutine(bStatus);
}

