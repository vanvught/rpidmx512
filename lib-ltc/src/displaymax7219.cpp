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

#include <stdint.h>
#include <assert.h>

#include "max72197segment.h"
#include "max7219matrix.h"

#include "displaymax7219.h"


DisplayMax7219 *DisplayMax7219::s_pThis = 0;

DisplayMax7219::DisplayMax7219(tMax7219Types tType, bool bShowSysTime):
	m_pMax7219Set(0),
	m_bShowSysTime(bShowSysTime)
{
	s_pThis = this;

	if (tType == MAX7219_7SEGMENT) {
		m_pMax7219Set = new Max72197Segment;
	} else {
		m_pMax7219Set = new Max7219Matrix;
	}

	assert(m_pMax7219Set != 0);
}

DisplayMax7219::~DisplayMax7219(void) {
	delete m_pMax7219Set;
	m_pMax7219Set = 0;
}

void DisplayMax7219::Init(uint8_t nIntensity) {
	m_pMax7219Set->Init(nIntensity);
}

void DisplayMax7219::Show(const char* pTimecode) {
	m_pMax7219Set->Show(pTimecode);
}

void DisplayMax7219::ShowSysTime(void) {
	if (m_bShowSysTime) {
		m_pMax7219Set->ShowSysTime();
	}
}
