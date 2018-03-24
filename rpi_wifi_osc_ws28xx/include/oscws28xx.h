/**
 * @file oscws28xx.h
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

#ifndef OSCWS28XX_H_
#define OSCWS28XX_H_

#include <stdint.h>

#include "ws28xxstripe.h"

#define FRAME_BUFFER_SIZE	1024

class OSCWS28xx  {
public:
	OSCWS28xx(unsigned, unsigned, const TWS28XXType, const char *);
	~OSCWS28xx(void);

	void Start(void);
	void Stop(void);

	void Run(void);

private:
	WS28XXStripe	*m_pLEDStripe;
	char m_Os[32];
	const char *m_pModel;
	const char *m_pSoC;
	int m_OutgoingPort;
	unsigned int m_nLEDCount;
	TWS28XXType m_nLEDType;
	const char *m_LEDType;
	uint8_t m_packet[FRAME_BUFFER_SIZE];
	bool m_Blackout;

	uint8_t	m_RGBWColour[4];
};

#endif /* OSCWS28XX_H_ */
