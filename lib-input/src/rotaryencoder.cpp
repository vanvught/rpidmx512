/**
 * @file rotaryencoder.cpp
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
/*
 * Inspired by http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
 */

#include <cstdint>

#include "rotaryencoder.h"

#include "debug.h"

namespace hs {	// half-step
static constexpr uint8_t START = 0x0;
static constexpr uint8_t CCW_BEGIN = 0x1;
static constexpr uint8_t CW_BEGIN = 0x2;
static constexpr uint8_t START_M = 0x3;
static constexpr uint8_t CW_BEGIN_M = 0x4;
static constexpr uint8_t CCW_BEGIN_M = 0x5;
}  // namespace hs

namespace fs {	// full-step
static constexpr uint8_t START = 0x0;
static constexpr uint8_t CW_FINAL = 0x1;
static constexpr uint8_t CW_BEGIN = 0x2;
static constexpr uint8_t CW_NEXT = 0x3;
static constexpr uint8_t CCW_BEGIN = 0x4;
static constexpr uint8_t CCW_FINAL = 0x5;
static constexpr uint8_t CCW_NEXT = 0x6;
}  // namespace fs

static constexpr uint8_t s_StateTableHalfStep[6][4] = {
  {hs::START_M,            			hs::CW_BEGIN,	 hs::CCW_BEGIN,  hs::START},
  {hs::START_M | RotaryEncoder::CW, hs::START,		 hs::CCW_BEGIN,  hs::START},
  {hs::START_M | RotaryEncoder::CCW,hs::CW_BEGIN,    hs::START,      hs::START},
  {hs::START_M,            			hs::CCW_BEGIN_M, hs::CW_BEGIN_M, hs::START},
  {hs::START_M,            			hs::START_M,     hs::CW_BEGIN_M, hs::START | RotaryEncoder::CCW},
  {hs::START_M,            			hs::CCW_BEGIN_M, hs::START_M,    hs::START | RotaryEncoder::CW},
};

static constexpr uint8_t s_StateTableFullStep[7][4] = {
  {fs::START,	 fs::CW_BEGIN,  fs::CCW_BEGIN, fs::START},
  {fs::CW_NEXT,  fs::START,     fs::CW_FINAL,  fs::START | RotaryEncoder::CCW},
  {fs::CW_NEXT,  fs::CW_BEGIN,  fs::START,     fs::START},
  {fs::CW_NEXT,  fs::CW_BEGIN,  fs::CW_FINAL,  fs::START},
  {fs::CCW_NEXT, fs::START,     fs::CCW_BEGIN, fs::START},
  {fs::CCW_NEXT, fs::CCW_FINAL, fs::START,     fs::START | RotaryEncoder::CW},
  {fs::CCW_NEXT, fs::CCW_FINAL, fs::CCW_BEGIN, fs::START},
};

uint8_t RotaryEncoder::Process(const uint8_t nInputAB) {
	DEBUG_PRINTF("%x:%x", m_nState & 0x0F, nInputAB & 0x03);

	if (m_bHalfStep) {
		m_nState = s_StateTableHalfStep[m_nState & 0x0F][nInputAB & 0x03];
	} else {
		m_nState = s_StateTableFullStep[m_nState & 0x0F][nInputAB & 0x03];
	}

	DEBUG_PRINTF("%x", m_nState & 0x30);

	return m_nState & 0x30;
}
