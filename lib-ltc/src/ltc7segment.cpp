/**
 * @file ltc7segment.cpp.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ltc7segment.h"
#include "ltc.h"

#include "display.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

Ltc7segment *Ltc7segment::s_pThis = 0;

const TDisplay7SegmentMessages msg[4] ALIGNED = {
		DISPLAY_7SEGMENT_MSG_LTC_FILM,
		DISPLAY_7SEGMENT_MSG_LTC_EBU,
		DISPLAY_7SEGMENT_MSG_LTC_DF,
		DISPLAY_7SEGMENT_MSG_LTC_SMPTE };

Ltc7segment::Ltc7segment(void) {
	assert(s_pThis == 0);
	s_pThis = this;
}

Ltc7segment::~Ltc7segment(void) {
}

void Ltc7segment::Show(TTimecodeTypes tTimecodeType) {
	if (tTimecodeType < TC_TYPE_UNKNOWN) {
		Display::Get()->Status(msg[tTimecodeType]);
	} else {
		Display::Get()->Status(DISPLAY_7SEGMENT_MSG_LTC_WAITING);
	}
}

