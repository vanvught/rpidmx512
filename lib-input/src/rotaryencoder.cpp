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

enum TState {
	STATE_START = 0x0,
	STATE_CW_FINAL = 0x1,
	STATE_CW_BEGIN = 0x2,
	STATE_CW_NEXT = 0x3,
	STATE_CCW_BEGIN = 0x4,
	STATE_CCW_FINAL = 0x5,
	STATE_CCW_NEXT = 0x6
};

static const unsigned char s_StateTable[7][4] = {
  // R_START
  {STATE_START,    STATE_CW_BEGIN,  STATE_CCW_BEGIN, STATE_START},
  // R_CW_FINAL
  {STATE_CW_NEXT,  STATE_START,     STATE_CW_FINAL,  STATE_START | ROTARY_DIRECTION_CW},
  // R_CW_BEGIN
  {STATE_CW_NEXT,  STATE_CW_BEGIN,  STATE_START,     STATE_START},
  // R_CW_NEXT
  {STATE_CW_NEXT,  STATE_CW_BEGIN,  STATE_CW_FINAL,  STATE_START},
  // R_CCW_BEGIN
  {STATE_CCW_NEXT, STATE_START,     STATE_CCW_BEGIN, STATE_START},
  // R_CCW_FINAL
  {STATE_CCW_NEXT, STATE_CCW_FINAL, STATE_START,     STATE_START | ROTARY_DIRECTION_CCW},
  // R_CCW_NEXT
  {STATE_CCW_NEXT, STATE_CCW_FINAL, STATE_CCW_BEGIN, STATE_START},
};

RotaryEncoder::RotaryEncoder(void) : m_nState(STATE_START) {
}

RotaryEncoder::~RotaryEncoder(void) {
}

TRotaryDirection RotaryEncoder::Process(uint8_t nInputAB) {
	DEBUG_PRINTF("%x:%x", static_cast<int>(m_nState & 0x0F), static_cast<int>(nInputAB & 0x03));
	m_nState = s_StateTable[m_nState & 0x0F][nInputAB & 0x03];
	DEBUG_PRINTF("%x", static_cast<int>(m_nState & 0x30));
	return static_cast<TRotaryDirection>((m_nState & 0x30));
}
