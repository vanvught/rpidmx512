/**
 * @file ltcdisplaymax7219matrix.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LTCDISPLAYMAX7219MATRIX_H_
#define LTCDISPLAYMAX7219MATRIX_H_

#include <stdint.h>

#include "ltcdisplaymax7219set.h"

#include "max7219matrix.h"

#define SEGMENTS	8

class LtcDisplayMax7219Matrix final: public LtcDisplayMax7219Set, public Max7219Matrix {
public:
	LtcDisplayMax7219Matrix();

	void Init(uint8_t nIntensity) override;

	void Show(const char *pTimecode) override;
	void ShowSysTime(const char *pSystemTime) override;

	void WriteChar(uint8_t nChar, uint8_t nPos=0) override;

	static LtcDisplayMax7219Matrix* Get() {
		return s_pThis;
	}

private:
	int32_t Offset(const char nChar, const char nSeconds);

private:
	char m_aBuffer[SEGMENTS];

	static LtcDisplayMax7219Matrix *s_pThis;
};

#endif /* LTCDISPLAYMAX7219MATRIX_H_ */
