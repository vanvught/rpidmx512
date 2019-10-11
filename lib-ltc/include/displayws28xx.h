/**
 * @file displayws28xx.h
 */
/* 
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Based on: displaymax7219.h
 * Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DISPLAYWS28XX_H_
#define DISPLAYWS28XX_H_

#include <stdint.h>
#include <stdbool.h>

#include "ws28xx.h"

#define WS28XX_LED_COUNT  ((3 * 7 * 8) + (3 * 2))

class DisplayWS28xx {
public:
	DisplayWS28xx(TWS28XXType tLedType, bool bShowSysTime = false);
	~DisplayWS28xx(void);

	void Init(uint8_t nIntensity);
	void Blackout();

	void Show(const char *pTimecode);
	void ShowSysTime(void);

	void WriteChar(uint8_t nChar, uint8_t nPos = 0);

	static DisplayWS28xx* Get(void) {
		return s_pThis;
	}

private:
	WS28xx *m_WS28xx;
	bool m_bShowSysTime;

	static DisplayWS28xx *s_pThis;
};

#endif /* DISPLAYWS28XX_H_ */
