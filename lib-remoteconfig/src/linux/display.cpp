/**
 * @file display.cpp
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

#if (__GNUC__ > 8) || defined(__clang__)
 #pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <stdint.h>
#include <assert.h>

#include "display.h"

#include "debug.h"

Display *Display::s_pThis = 0;

Display::Display(uint32_t nCols, uint32_t nRows):
	m_tType(DISPLAY_TYPE_UNKNOWN),
	m_LcdDisplay(0),
	m_bIsSleep(false),
	m_bHave7Segment(false),
#if !defined(NO_HAL)
	m_nMillis(0),
#endif
	m_nSleepTimeout(0)
{
	DEBUG_ENTRY

	assert(s_pThis == 0);
	s_pThis = this;

	DEBUG_EXIT
}

Display::Display(TDisplayTypes tDisplayType):
	m_nCols(0),
	m_nRows(0),
	m_LcdDisplay(0),
	m_bIsSleep(false),
	m_bHave7Segment(false),
#if !defined(NO_HAL)
	m_nMillis(0),
#endif
	m_nSleepTimeout(0)
{
	DEBUG_ENTRY
	assert(s_pThis == 0);
	s_pThis = this;

	DEBUG_EXIT
}

Display::~Display(void) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void Display::Cls(void) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void Display::TextStatus(const char *pText, TDisplay7SegmentMessages nStatus) {
	DEBUG_PUTS(pText);
}

uint8_t Display::Printf(uint8_t nLine, char const*, ...){
	DEBUG_ENTRY
	DEBUG_EXIT
	return 0;
}

uint8_t Display::Write(uint8_t nLine, const char *pText) {
	DEBUG_PUTS(pText);
	return 0;
}

void Display::ClearLine(unsigned char){
	DEBUG_ENTRY
	DEBUG_EXIT
}

#if !defined(NO_HAL)
void Display::SetSleep(bool bSleep) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void Display::Run(void) {
}
#endif
