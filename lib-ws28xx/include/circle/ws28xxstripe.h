/**
 * @file ws28xxstripe.h
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

#ifndef _ws28xxstripe_h
#define _ws28xxstripe_h

#include <circle/interrupt.h>
#include <circle/spimasterdma.h>
#include <circle/types.h>

enum TWS28XXType {
	WS2801 = 0,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	SK6812,
	SK6812W
};

class CWS28XXStripe
{
public:
	// nClockSpeed is only variable on WS2801, otherwise ignored
	CWS28XXStripe (CInterruptSystem *pInterruptSystem, TWS28XXType Type, unsigned nLEDCount, unsigned nClockSpeed = 4000000);
	~CWS28XXStripe (void);

	boolean Initialize (void);

	unsigned GetLEDCount (void) const;
	TWS28XXType GetLEDType(void) const;

	// must not be called when DMA operation is active
	void SetLED (unsigned nLEDIndex, u8 nRed, u8 nGreen, u8 nBlue);	// nIndex is 0-based
	void SetLED (unsigned nLEDIndex, u8 nRed, u8 nGreen, u8 nBlue, u8 nWhite);	// nIndex is 0-based

	// must not be called when DMA operation is active
	void Update (void);
	void Blackout (void);

	// returns TRUE while DMA operation is active
	boolean IsUpdating (void) const;

private:
	void SetColorWS28xx (unsigned nOffset, u8 nValue);

private:
	void SPICompletionRoutine (boolean bStatus);
	static void SPICompletionStub (boolean bStatus, void *pParam);

private:
	TWS28XXType			m_Type;
	unsigned			m_nLEDCount;
	unsigned	 		m_nBufSize;
	u8					*m_pBuffer;
	u8					*m_pReadBuffer;
	u8					*m_pBlackoutBuffer;
	volatile boolean 	m_bUpdating;
	CSPIMasterDMA	 	m_SPIMaster;
};

#endif
