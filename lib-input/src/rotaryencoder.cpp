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

#include <stdint.h>

#include "rotaryencoder.h"

#include "debug.h"

struct State {
	static constexpr uint8_t START = 0x0;
	static constexpr uint8_t CW_FINAL = 0x1;
	static constexpr uint8_t CW_BEGIN = 0x2;
	static constexpr uint8_t CW_NEXT = 0x3;
	static constexpr uint8_t CCW_BEGIN = 0x4;
	static constexpr uint8_t CCW_FINAL = 0x5;
	static constexpr uint8_t CCW_NEXT = 0x6;
};

static constexpr uint8_t s_StateTable[][4] = {
  // R_START
  {State::START,	State::CW_BEGIN,  State::CCW_BEGIN, State::START},
  // R_CW_FINAL
  {State::CW_NEXT,  State::START,     State::CW_FINAL,  State::START | RotaryEncoder::CW},
  // R_CW_BEGIN
  {State::CW_NEXT,  State::CW_BEGIN,  State::START,     State::START},
  // R_CW_NEXT
  {State::CW_NEXT,  State::CW_BEGIN,  State::CW_FINAL,  State::START},
  // R_CCW_BEGIN
  {State::CCW_NEXT, State::START,     State::CCW_BEGIN, State::START},
  // R_CCW_FINAL
  {State::CCW_NEXT, State::CCW_FINAL, State::START,     State::START | RotaryEncoder::CCW},
  // R_CCW_NEXT
  {State::CCW_NEXT, State::CCW_FINAL, State::CCW_BEGIN, State::START},
};

RotaryEncoder::RotaryEncoder() : m_nState(State::START) {
}

uint8_t RotaryEncoder::Process(uint8_t nInputAB) {
	DEBUG_PRINTF("%x:%x", m_nState & 0x0F, nInputAB & 0x03);

	m_nState = s_StateTable[m_nState & 0x0F][nInputAB & 0x03];

	DEBUG_PRINTF("%x", m_nState & 0x30);

	return m_nState & 0x30;
}
