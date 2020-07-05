/**
 * @file ltcdisplaymax7219.h
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

#ifndef LTCDISPLAYMAX7219_H_
#define LTCDISPLAYMAX7219_H_

#include <stdint.h>

#include "ltcdisplaymax7219set.h"

enum TLtcDisplayMax7219Types {
	LTCDISPLAYMAX7219_TYPE_MATRIX,
	LTCDISPLAYMAX7219_TYPE_7SEGMENT
};

class LtcDisplayMax7219 {
public:
	LtcDisplayMax7219(TLtcDisplayMax7219Types tType = LTCDISPLAYMAX7219_TYPE_MATRIX);
	~LtcDisplayMax7219();

	void Init(uint8_t nIntensity);

	void Print();

	void Show(const char *pTimecode);
	void ShowSysTime(const char *pSystemTime);

	void WriteChar(uint8_t nChar, uint8_t nPos = 0);

	static LtcDisplayMax7219 *Get() {
		return s_pThis;
	}

private:
	TLtcDisplayMax7219Types m_tMax7219Types;
	uint8_t m_nIntensity = 0;
	LtcDisplayMax7219Set *m_pMax7219Set = nullptr;

	static LtcDisplayMax7219 *s_pThis;
};

#endif /* LTCDISPLAYMAX7219_H_ */
