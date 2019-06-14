/**
 * @file displaymatrix.h
 *
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

#ifndef DISPLAYMATRIX_H_
#define DISPLAYMATRIX_H_

#include <stdint.h>

#include "max7219set.h"

#include "device_info.h"

#define SEGMENTS	8

class Max7219Matrix: public Max7219Set {
public:
	Max7219Matrix(void);
	~Max7219Matrix(void);

	void Init(uint8_t nIntensity);

	void Show(const char *pTimecode);
	void ShowSysTime(void);

	static Max7219Matrix* Get(void) {
		return s_pThis;
	}

private:
	device_info_t m_DeviceInfo;
	uint8_t m_aBuffer[SEGMENTS];
	uint32_t m_nSecondsPrevious;

	static Max7219Matrix *s_pThis;
};

#endif /* DISPLAYMATRIX_H_ */
