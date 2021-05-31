/**
 * @file max7219matrix.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DEVICE_MAX7219MATRIX_H_
#define DEVICE_MAX7219MATRIX_H_

#include <cstdint>

#include "max7219.h"

class Max7219Matrix: public MAX7219 {
public:
	Max7219Matrix();
	~Max7219Matrix();

	void Init(uint16_t nCount, uint8_t nIntensity);

	void Cls();

	void Write(const char *pBuffer, uint16_t nCount);

	void UpdateCharacter(uint32_t nChar, const uint8_t pBytes[8]);

private:
	uint8_t Rotate(uint32_t r, uint32_t x);
	void WriteAll(uint8_t nRegister, uint8_t nData);

private:
	uint32_t m_nFontSize;
	uint8_t *m_pFont;
	uint16_t m_nCount { 4 };
};

#endif /* DEVICE_MAX7219MATRIX_H_ */
