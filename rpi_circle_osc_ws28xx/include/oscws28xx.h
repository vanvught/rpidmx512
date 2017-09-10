/**
 * @file oscws28xx.h
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <circle/fs/fat/fatfs.h>

#include "ws28xxstripe.h"

#include "oscserver.h"

class COSCWS28xx: public OSCServer
{
public:
	COSCWS28xx(CInterruptSystem*, CDevice *, CFATFileSystem *, unsigned);
	~COSCWS28xx(void);

private:
	void MessageReceived(u8 *, int, u32);

private:
	CInterruptSystem	*m_pInterrupt;
	CDevice				*m_pTarget;
	CMachineInfo 		m_MachineInfo;
	CWS28XXStripe		*m_pLEDStripe;
	TWS28XXType			m_LEDType;
	unsigned			m_nLEDCount;
	CPropertiesFile		m_Properties;
	boolean 			m_Blackout;

	u8 					m_RGBWColour[4];
};

#endif
