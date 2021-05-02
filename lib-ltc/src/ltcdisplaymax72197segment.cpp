/**
 * @file ltcdisplaymax72197segment.cpp
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
#include <time.h>
#include <cassert>

#include "ltcdisplaymax72197segment.h"

#include "max72197segment.h"

LtcDisplayMax72197Segment *LtcDisplayMax72197Segment::s_pThis = nullptr;

LtcDisplayMax72197Segment::LtcDisplayMax72197Segment() {
	assert(s_pThis == nullptr);
	s_pThis = this;
}

void LtcDisplayMax72197Segment::Init(uint8_t nIntensity) {
	Max72197Segment::Init(nIntensity);

	WriteRegister(max7219::reg::DIGIT6, 0x80);
	WriteRegister(max7219::reg::DIGIT4, 0x80, false);
	WriteRegister(max7219::reg::DIGIT2, 0x80, false);
}

void LtcDisplayMax72197Segment::Show(const char *pTimecode) {
	WriteRegister(max7219::reg::DIGIT7, static_cast<uint32_t>(pTimecode[0] - '0'));
	WriteRegister(max7219::reg::DIGIT6, static_cast<uint32_t>((pTimecode[1] - '0') | 0x80), false);
	WriteRegister(max7219::reg::DIGIT5, static_cast<uint32_t>(pTimecode[3] - '0'), false);
	WriteRegister(max7219::reg::DIGIT4, static_cast<uint32_t>((pTimecode[4] - '0') | 0x80), false);
	WriteRegister(max7219::reg::DIGIT3, static_cast<uint32_t>(pTimecode[6] - '0'), false);
	WriteRegister(max7219::reg::DIGIT2, static_cast<uint32_t>((pTimecode[7] - '0') | 0x80), false);
	WriteRegister(max7219::reg::DIGIT1, static_cast<uint32_t>(pTimecode[9] - '0'), false);
	WriteRegister(max7219::reg::DIGIT0, static_cast<uint32_t>(pTimecode[10] - '0'), false);
}

void LtcDisplayMax72197Segment::ShowSysTime(const char *pSystemTime) {
	WriteRegister(max7219::reg::DIGIT7, max7219::digit::BLANK);
	WriteRegister(max7219::reg::DIGIT6, static_cast<uint32_t>(pSystemTime[0] - '0'), false);
	WriteRegister(max7219::reg::DIGIT5, static_cast<uint32_t>((pSystemTime[1] - '0') | 0x80), false);
	WriteRegister(max7219::reg::DIGIT4, static_cast<uint32_t>(pSystemTime[3] - '0'), false);
	WriteRegister(max7219::reg::DIGIT3, static_cast<uint32_t>((pSystemTime[4] - '0') | 0x80), false);
	WriteRegister(max7219::reg::DIGIT2, static_cast<uint32_t>(pSystemTime[6] - '0'), false);
	WriteRegister(max7219::reg::DIGIT1, static_cast<uint32_t>(pSystemTime[7] - '0'), false);
	WriteRegister(max7219::reg::DIGIT0, max7219::digit::BLANK, false);
}

void LtcDisplayMax72197Segment::WriteChar(uint8_t nChar, uint8_t nPos) {
	if (nPos < 8) {
		WriteRegister(static_cast<uint32_t>(max7219::reg::DIGIT0 + nPos), nChar);
	}
}
