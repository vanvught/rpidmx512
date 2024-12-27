/**
 * @file ltcdisplaymax7219.h
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcdisplaymax7219set.h"

#include "ltcdisplaymax72197segment.h"
#include "ltcdisplaymax7219matrix.h"



namespace ltc::display::max7219 {
enum class Types {
	MATRIX, SEGMENT
};
} // namespace ltc::display::max7219



class LtcDisplayMax7219 {
public:
	LtcDisplayMax7219(ltc::display::max7219::Types type): m_type(type) {
		assert(s_pThis == nullptr);
		s_pThis = this;

		if (m_type == ltc::display::max7219::Types::SEGMENT) {
			m_pMax7219Set = new LtcDisplayMax72197Segment;
		} else {
			m_pMax7219Set = new LtcDisplayMax7219Matrix;
		}

		assert(m_pMax7219Set != nullptr);
	}

	void Init(uint8_t nIntensity) {
		m_nIntensity = nIntensity;
		m_pMax7219Set->Init(nIntensity);
	}

	void Show(const char *pTimecode) {
		m_pMax7219Set->Show(pTimecode);
	}

	void ShowSysTime(const char *pSystemTime) {
		m_pMax7219Set->ShowSysTime(pSystemTime);
	}

	void WriteChar(uint8_t nChar, uint8_t nPos = 0) {
		m_pMax7219Set->WriteChar(nChar, nPos);
	}

	void Print() {
		printf("MAX7219\n");
		printf(" %s [%d]\n", m_type == ltc::display::max7219::Types::SEGMENT ? "7-segment" : "matrix", m_nIntensity);
	}

	static LtcDisplayMax7219* Get() {
		return s_pThis;
	}

private:
	ltc::display::max7219::Types m_type;
	LtcDisplayMax7219Set *m_pMax7219Set;
	uint8_t m_nIntensity { 0 };

	static LtcDisplayMax7219 *s_pThis;
};

#endif /* LTCDISPLAYMAX7219_H_ */
