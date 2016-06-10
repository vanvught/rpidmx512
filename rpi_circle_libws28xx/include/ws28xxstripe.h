//
// ws28xxstripe.h
//
// Driver for WS28XX controlled LED stripes
// Original development by Arjan van Vught <info@raspberrypi-dmx.nl>
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _ws28xxstripe_h
#define _ws28xxstripe_h

#include <circle/interrupt.h>
#include <circle/spimasterdma.h>
#include <circle/types.h>

enum TWS28XXType
{
	WS2801,
	WS2812,
	WS2812B
};

class CWS28XXStripe
{
public:
	// nClockSpeed is only variable on WS2801, otherwise ignored
	CWS28XXStripe (CInterruptSystem *pInterruptSystem,
		       TWS28XXType Type, unsigned nLEDCount, unsigned nClockSpeed = 4000000);
	~CWS28XXStripe (void);

	boolean Initialize (void);

	unsigned GetLEDCount (void) const;

	// must not be called when DMA operation is active
	void SetLED (unsigned nLEDIndex, u8 nRed, u8 nGreen, u8 nBlue);		// nIndex is 0-based

	// must not be called when DMA operation is active
	void Update (void);
	void Blackout (void);		// temporary switch all LEDs off

	// returns TRUE while DMA operation is active
	boolean IsUpdating (void) const;

private:
	void SetColorWS2812 (unsigned nOffset, u8 nValue);

	void SPICompletionRoutine (boolean bStatus);
	static void SPICompletionStub (boolean bStatus, void *pParam);

private:
	TWS28XXType	 m_Type;
	unsigned	 m_nLEDCount;
	unsigned	 m_nBufSize;
	u8		*m_pBuffer;
	u8		*m_pReadBuffer;
	u8		*m_pBlackoutBuffer;
	volatile boolean m_bUpdating;
	CSPIMasterDMA	 m_SPIMaster;
};

#endif
