/**
 * @file rotaryencoder.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
 #include "firmware/debug/debug_debug.h"

namespace hs
{ // half-step
static constexpr uint8_t kStart = 0x0;
static constexpr uint8_t kCcwBegin = 0x1;
static constexpr uint8_t kCwBegin = 0x2;
static constexpr uint8_t kStartM = 0x3;
static constexpr uint8_t kCwBeginM = 0x4;
static constexpr uint8_t kCcwBeginM = 0x5;
} // namespace hs

namespace fs
{ // full-step
static constexpr uint8_t kStart = 0x0;
static constexpr uint8_t kCwFinal = 0x1;
static constexpr uint8_t kCwBegin = 0x2;
static constexpr uint8_t kCwNext = 0x3;
static constexpr uint8_t kCcwBegin = 0x4;
static constexpr uint8_t kCcwFinal = 0x5;
static constexpr uint8_t kCcwNext = 0x6;
} // namespace fs

static constexpr uint8_t kStateTableHalfStep[6][4] = {
  {hs::kStartM,            			hs::kCwBegin,	 hs::kCcwBegin,  hs::kStart},
  {hs::kStartM | RotaryEncoder::CW, hs::kStart,		 hs::kCcwBegin,  hs::kStart},
  {hs::kStartM | RotaryEncoder::CCW,hs::kCwBegin,    hs::kStart,      hs::kStart},
  {hs::kStartM,            			hs::kCcwBeginM, hs::kCwBeginM, hs::kStart},
  {hs::kStartM,            			hs::kStartM,     hs::kCwBeginM, hs::kStart | RotaryEncoder::CCW},
  {hs::kStartM,            			hs::kCcwBeginM, hs::kStartM,    hs::kStart | RotaryEncoder::CW},
};

static constexpr uint8_t kStateTableFullStep[7][4] = {
  {fs::kStart,	 fs::kCwBegin,  fs::kCcwBegin, fs::kStart},
  {fs::kCwNext,  fs::kStart,     fs::kCwFinal,  fs::kStart | RotaryEncoder::CCW},
  {fs::kCwNext,  fs::kCwBegin,  fs::kStart,     fs::kStart},
  {fs::kCwNext,  fs::kCwBegin,  fs::kCwFinal,  fs::kStart},
  {fs::kCcwNext, fs::kStart,     fs::kCcwBegin, fs::kStart},
  {fs::kCcwNext, fs::kCcwFinal, fs::kStart,     fs::kStart | RotaryEncoder::CW},
  {fs::kCcwNext, fs::kCcwFinal, fs::kCcwBegin, fs::kStart},
};

uint8_t RotaryEncoder::Process(uint8_t input_ab)
{
    DEBUG_PRINTF("%x:%x", state_ & 0x0F, input_ab & 0x03);

    if (halfstep_)
    {
        state_ = kStateTableHalfStep[state_ & 0x0F][input_ab & 0x03];
    }
    else
    {
        state_ = kStateTableFullStep[state_ & 0x0F][input_ab & 0x03];
    }

    DEBUG_PRINTF("%x", state_ & 0x30);

    return state_ & 0x30;
}
