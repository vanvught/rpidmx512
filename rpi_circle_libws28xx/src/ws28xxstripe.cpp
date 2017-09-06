/**
 * @file ws28xxstripe.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2016  R. Stange <rsta2@o2online.de>
 * Based on https://github.com/rsta2/circle/tree/master/addon/WS28XX
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <circle/logger.h>
#include <circle/util.h>
#include <assert.h>

#include "ws28xxstripe.h"

/**
 *
 * @param pInterruptSystem
 * @param Type
 * @param nLEDCount
 * @param nClockSpeed
 */
CWS28XXStripe::CWS28XXStripe (CInterruptSystem *pInterruptSystem, TWS28XXType Type, unsigned nLEDCount, unsigned nClockSpeed)
:	m_Type (Type),
	m_nLEDCount (nLEDCount),
	m_bUpdating (FALSE),
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

/**
 *
 */
CWS28XXStripe::~CWS28XXStripe (void)
{
	while (m_bUpdating)
	{
		// just wait
	}

	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pReadBuffer;
	m_pReadBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;
}

/**
 *
 * @return
 */
boolean CWS28XXStripe::Initialize (void)
{
	return m_SPIMaster.Initialize ();
}

/**
 *
 */
unsigned CWS28XXStripe::GetLEDCount (void) const
{
	return m_nLEDCount;
}

/**
 *
 * @param nLEDIndex
 * @param nRed
 * @param nGreen
 * @param nBlue
 */
void CWS28XXStripe::SetLED(unsigned nLEDIndex, u8 nRed, u8 nGreen, u8 nBlue) {
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

void CWS28XXStripe::SetLED(unsigned nLEDIndex, u8 nRed, u8 nGreen, u8 nBlue, u8 nWhite) {
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

/**
 *
 */
void CWS28XXStripe::Update (void)
{
	assert (!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine (SPICompletionStub, this);

	assert (m_pBuffer != 0);
	assert (m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead (0, m_pBuffer, m_pReadBuffer, m_nBufSize);
}

/**
 *
 */
void CWS28XXStripe::Blackout (void)
{
	assert (!m_bUpdating);
	m_bUpdating = TRUE;

	m_SPIMaster.SetCompletionRoutine (SPICompletionStub, this);

	assert (m_pBlackoutBuffer != 0);
	assert (m_pReadBuffer != 0);
	m_SPIMaster.StartWriteRead (0, m_pBlackoutBuffer, m_pReadBuffer, m_nBufSize);
}

/**
 *
 * @return
 */
boolean CWS28XXStripe::IsUpdating (void) const
{
	return m_bUpdating;
}

/**
 *
 * @param nOffset
 * @param nValue
 */
void CWS28XXStripe::SetColorWS28xx(unsigned nOffset, u8 nValue) {
	assert(m_Type != WS2801);
	u8 nHighCode = m_Type == WS2812B ? 0xF8 : 0xF0;

	assert(nOffset + 7 < m_nBufSize);

	for (u8 nMask = 0x80; nMask != 0; nMask >>= 1) {
		if (nValue & nMask) {
			m_pBuffer[nOffset] = nHighCode;
		} else {
			m_pBuffer[nOffset] = 0xC0;
		}

		nOffset++;
	}
}

/**
 *
 * @param bStatus
 */
void CWS28XXStripe::SPICompletionRoutine (boolean bStatus)
{
	if (!bStatus)
	{
		CLogger::Get ()->Write ("ws28xx", LogError, "SPI DMA operation failed");
	}

	assert (m_bUpdating);
	m_bUpdating = FALSE;
}

/**
 *
 * @param bStatus
 * @param pParam
 */
void CWS28XXStripe::SPICompletionStub (boolean bStatus, void *pParam)
{
	CWS28XXStripe *pThis = (CWS28XXStripe *) pParam;
	assert (pThis != 0);

	pThis->SPICompletionRoutine (bStatus);
}

