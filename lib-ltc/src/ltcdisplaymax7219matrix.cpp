/**
 * @file ltcdisplaymax7219matrix.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <cassert>

#include "ltcdisplaymax7219matrix.h"

#include "max7219matrix.h"

LtcDisplayMax7219Matrix *LtcDisplayMax7219Matrix::s_pThis = 0;

LtcDisplayMax7219Matrix::LtcDisplayMax7219Matrix(void) {
	assert(s_pThis == 0);
	s_pThis = this;
}

LtcDisplayMax7219Matrix::~LtcDisplayMax7219Matrix(void) {
}

void LtcDisplayMax7219Matrix::Init(uint8_t nIntensity) {
	Max7219Matrix::Init(SEGMENTS, nIntensity);
	Write("Waiting", 7);
}

void LtcDisplayMax7219Matrix::Show(const char *pTimecode) {
	m_aBuffer[0] = pTimecode[0];
	m_aBuffer[1] = pTimecode[1];
	m_aBuffer[2] = pTimecode[3];
	m_aBuffer[3] = pTimecode[4];
	m_aBuffer[4] = pTimecode[6];
	m_aBuffer[5] = pTimecode[7];
	m_aBuffer[6] = pTimecode[9];
	m_aBuffer[7] = pTimecode[10];

	Write(m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::ShowSysTime(const char *pSystemTime) {
	m_aBuffer[0] = pSystemTime[0];
	m_aBuffer[1] = pSystemTime[1];
	m_aBuffer[2] = pSystemTime[3];
	m_aBuffer[3] = pSystemTime[4];
	m_aBuffer[4] = pSystemTime[6];
	m_aBuffer[5] = pSystemTime[7];

	Write(m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::WriteChar(__attribute__((unused)) uint8_t nChar, __attribute__((unused)) uint8_t nPos) {
	// TODO Max7219Matrix::WriteChar
}

