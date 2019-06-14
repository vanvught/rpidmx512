/**
 * @file displaymax7219.h
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DISPLAYMAX7219_H_
#define DISPLAYMAX7219_H_

#include <stdint.h>
#include <stdbool.h>

#include "max7219set.h"

enum tMax7219Types {
	MAX7219_MATRIX,
	MAX7219_7SEGMENT
};

class DisplayMax7219 {
public:
	DisplayMax7219(tMax7219Types tType = MAX7219_MATRIX, bool bShowSysTime = false);
	~DisplayMax7219(void);

	void Init(uint8_t nIntensity);

	void Show(const char *pTimecode);
	void ShowSysTime(void);

	 static DisplayMax7219* Get(void) {
		return s_pThis;
	}

private:
	Max7219Set *m_pMax7219Set;
	bool m_bShowSysTime;

	static DisplayMax7219 *s_pThis;
};


#endif /* DISPLAYMAX7219_H_ */
