/**
 * @file dmxconst.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMX_CONST_H_
#define DMX_CONST_H_

#include <cstdint>

namespace dmx {
enum class PortDirection {
	OUTP, INP
};
enum class OutputStyle {
	DELTA,
	CONTINOUS
};
static constexpr uint32_t START_CODE = 0;				   ///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".
namespace min {
static constexpr uint32_t CHANNELS = 2;
}  // namespace min
namespace max {
static constexpr uint32_t CHANNELS = 512;
}  // namespace max
namespace transmit {
static constexpr uint32_t BREAK_TIME_MIN = 92;				///< 92 us
static constexpr uint32_t BREAK_TIME_TYPICAL = 176;		    ///< 176 us
static constexpr uint32_t MAB_TIME_MIN = 12;				///< 12 us
static constexpr uint32_t MAB_TIME_MAX = 1000000;			///< 1s
static constexpr uint32_t REFRESH_RATE_DEFAULT = 40;		///< 40 Hz
static constexpr uint32_t PERIOD_DEFAULT = (1000000U / REFRESH_RATE_DEFAULT);///< 25000 us
static constexpr uint32_t BREAK_TO_BREAK_TIME_MIN = 1204;	///< us
}  // namespace transmit
}  // namespace dmx

#endif /* DMX_CONST_H_ */
