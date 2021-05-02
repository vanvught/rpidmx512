/**
 * @file ltcdisplaymax7219.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "ltcdisplaymax7219.h"

#include "ltcdisplaymax72197segment.h"
#include "ltcdisplaymax7219matrix.h"

LtcDisplayMax7219 *LtcDisplayMax7219::s_pThis = nullptr;

LtcDisplayMax7219::LtcDisplayMax7219(TLtcDisplayMax7219Types tType): m_tMax7219Types(tType) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	if (tType == LTCDISPLAYMAX7219_TYPE_7SEGMENT) {
		m_pMax7219Set = new LtcDisplayMax72197Segment;
	} else {
		m_pMax7219Set = new LtcDisplayMax7219Matrix;
	}

	assert(m_pMax7219Set != nullptr);
}

LtcDisplayMax7219::~LtcDisplayMax7219() {
	delete m_pMax7219Set;
	m_pMax7219Set = nullptr;
}

void LtcDisplayMax7219::Init(uint8_t nIntensity) {
	m_nIntensity = nIntensity;
	m_pMax7219Set->Init(nIntensity);
}

void LtcDisplayMax7219::Show(const char *pTimecode) {
	m_pMax7219Set->Show(pTimecode);
}

void LtcDisplayMax7219::ShowSysTime(const char *pSystemTime) {
	m_pMax7219Set->ShowSysTime(pSystemTime);
}

void LtcDisplayMax7219::WriteChar(uint8_t nChar, uint8_t nPos) {
	m_pMax7219Set->WriteChar(nChar, nPos);
}

void LtcDisplayMax7219::Print() {
	printf("MAX7219\n");
	printf(" %s [%d]\n", m_tMax7219Types == LTCDISPLAYMAX7219_TYPE_7SEGMENT ? "7-segment" : "matrix", m_nIntensity);
}
