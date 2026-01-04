/**
 * @file hal_panelled.h
 *
 */
/* Copyright (C) 2021-20245 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_HAL_PANELLED_H_
#define H3_HAL_PANELLED_H_

#include <cstdint>

namespace hal::panelled
{
inline constexpr uint32_t ACTIVITY = 0;
inline constexpr uint32_t ARTNET = 0;
inline constexpr uint32_t DDP = 0;
inline constexpr uint32_t SACN = 0;
inline constexpr uint32_t LTC_IN = 0;
inline constexpr uint32_t LTC_OUT = 0;
inline constexpr uint32_t MIDI_IN = 0;
inline constexpr uint32_t MIDI_OUT = 0;
inline constexpr uint32_t OSC_IN = 0;
inline constexpr uint32_t OSC_OUT = 0;
inline constexpr uint32_t TCNET = 0;
// DMX
inline constexpr uint32_t PORT_A_RX = 0;
inline constexpr uint32_t PORT_A_TX = 0;

inline void Init() {}
inline void On([[maybe_unused]] uint32_t on) {}
inline void Off([[maybe_unused]] uint32_t off) {}
inline void Run() {}
} // namespace hal::panelled

#endif  // H3_HAL_PANELLED_H_
