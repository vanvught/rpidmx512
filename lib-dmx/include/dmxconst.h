/**
 * @file dmxconst.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXCONST_H_
#define DMXCONST_H_

#include <cstdint>

namespace dmx
{
enum class PortDirection
{
    kInput,
    kOutput,
    kDisable
};

enum class OutputStyle
{
    kDelta,   ///< DMX frame is triggered
    kConstant ///< DMX output is continuous
};

enum class SendStyle
{
    kDirect,
    kSync
};

inline constexpr uint32_t kStartCode = 0; ///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".
inline constexpr uint32_t kChannelsMin = 2;
inline constexpr uint32_t kChannelsMax = 512;
namespace transmit
{
inline constexpr uint32_t kBreakTimeMin = 92;                                ///< 92 us
inline constexpr uint32_t kBreakTimeTypical = 176;                           ///< 176 us
inline constexpr uint32_t kMabTimeMin = 12;                                  ///< 12 us
inline constexpr uint32_t kMabTimeMax = 1000000;                             ///< 1s
inline constexpr uint32_t kRefreshRateDefault = 40;                          ///< 40 Hz
inline constexpr uint32_t kPeriodDefault = (1000000U / kRefreshRateDefault); ///< 25000 us
inline constexpr uint32_t kBreakToBreakTimeMin = 1204;                     ///< us
} // namespace transmit
} // namespace dmx

#endif  // DMXCONST_H_
