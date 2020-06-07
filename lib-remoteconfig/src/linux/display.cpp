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

#if defined(__clang__)
 #pragma GCC diagnostic ignored "-Wunused-private-field"
#endif

#include <stdint.h>
#include <cassert>

#include "display.h"

#include "debug.h"

Display *Display::s_pThis = 0;

Display::Display(__attribute__((unused)) uint32_t nCols, __attribute__((unused)) uint32_t nRows):
	m_tType(DisplayType::UNKNOWN),
	m_LcdDisplay(0),
	m_bIsSleep(false),
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

Display::Display(__attribute__((unused)) DisplayType tDisplayType):
	m_nCols(0),
	m_nRows(0),
	m_LcdDisplay(0),
	m_bIsSleep(false),
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

void Display::TextStatus(__attribute__((unused)) const char *pText) {
	DEBUG_PUTS(pText);
}

void Display::TextStatus(__attribute__((unused)) const char *pText, __attribute__((unused)) Display7SegmentMessage msg, __attribute__((unused)) uint32_t nConsoleColor) {
	DEBUG_PUTS(pText);
}
void Display::TextStatus(__attribute__((unused)) const char *pText, __attribute__((unused)) uint8_t nValue7Segment, __attribute__((unused)) bool bHex) {
	DEBUG_PUTS(pText);
}

int Display::Printf(__attribute__((unused)) uint8_t nLine, __attribute__((unused)) char const*, ...){
	DEBUG_ENTRY
	DEBUG_EXIT
	return 0;
}

int Display::Write(__attribute__((unused)) uint8_t nLine, __attribute__((unused)) const char *pText) {
	DEBUG_PUTS(pText);
	return 0;
}

void Display::ClearLine(__attribute__((unused)) unsigned char){
	DEBUG_ENTRY
	DEBUG_EXIT
}

#if !defined(NO_HAL)
void Display::SetSleep(__attribute__((unused)) bool bSleep) {
	DEBUG_ENTRY
	DEBUG_EXIT
}

void Display::Run(void) {
}
#endif
