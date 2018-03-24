/**
 * @file oscws28xx.h
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef _oscws2801_h
#define _oscws2801_h

#include <circle/interrupt.h>
#include <circle/device.h>
#include <circle/machineinfo.h>

#include "ws28xxstripeparams.h"
#include "ws28xxstripe.h"

#ifndef FRAME_BUFFER_SIZE
#define FRAME_BUFFER_SIZE	1024
#endif

class COSCWS28xx
{
public:
	COSCWS28xx(CInterruptSystem*, CDevice *, unsigned);
	~COSCWS28xx(void);

	void Start(void);
	void Stop(void);

	void Run(void);

private:
	CInterruptSystem	*m_pInterrupt;
	CDevice				*m_pTarget;
	unsigned			m_nRemotePort;
	CMachineInfo 		m_MachineInfo;
	WS28XXStripe		*m_pLEDStripe;
	TWS28XXType			m_LEDType;
	unsigned			m_nLEDCount;
	boolean 			m_Blackout;
	u8 					m_RGBWColour[4];
	WS28XXStripeParams	m_DeviceParams;
	uint8_t 			m_packet[FRAME_BUFFER_SIZE];
};

#endif
