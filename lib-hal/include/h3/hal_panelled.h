/**
 * @file hal_panelled.h
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

namespace hal::panelled {
inline constexpr uint32_t kActivity = 0;
inline constexpr uint32_t kArtnet = 0;
inline constexpr uint32_t kDdp = 0;
inline constexpr uint32_t kSacn = 0;
inline constexpr uint32_t kLtcIn = 0;
inline constexpr uint32_t kLtcOut = 0;
inline constexpr uint32_t kMidiIn = 0;
inline constexpr uint32_t kMidiOut = 0;
inline constexpr uint32_t kOscIn = 0;
inline constexpr uint32_t kOscOut = 0;
inline constexpr uint32_t kTcnet = 0;
// DMX
inline constexpr uint32_t kPortARx = 0;
inline constexpr uint32_t kPortATx = 0;
// RDM
inline constexpr uint32_t kPortARdm = 0;

inline void Init() {}
inline void On([[maybe_unused]] uint32_t on) {}
inline void Off([[maybe_unused]] uint32_t off) {}
inline void Run() {}
} // namespace hal::panelled

#endif // H3_HAL_PANELLED_H_
