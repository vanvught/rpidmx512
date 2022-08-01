/**
 * @file dmxconst.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

namespace dmx {
enum class PortDirection {
	OUTP, INP, DISABLED
};
static constexpr auto START_CODE = 0U;				///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".
namespace min {
static constexpr auto CHANNELS = 2U;
}  // namespace min
namespace max {
static constexpr auto CHANNELS = 512U;
}  // namespace max
namespace transmit {
static constexpr auto BREAK_TIME_MIN = 92U;				///< 92 us
static constexpr auto BREAK_TIME_TYPICAL = 176U;		///< 176 us
static constexpr auto MAB_TIME_MIN = 12U;				///< 12 us
static constexpr auto MAB_TIME_MAX = 1000000U;			///< 1s
static constexpr auto REFRESH_RATE_DEFAULT = 40U;		///< 40 Hz
static constexpr auto PERIOD_DEFAULT = (1000000U / REFRESH_RATE_DEFAULT);///< 25000 us
static constexpr auto BREAK_TO_BREAK_TIME_MIN = 1204U;	///< us
}  // namespace transmit
}  // namespace dmx

#endif /* DMX_CONST_H_ */
