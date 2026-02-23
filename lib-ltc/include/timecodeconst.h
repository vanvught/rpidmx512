/**
 * @file timecodeconst.h
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

#ifndef TIMECODECONST_H_
#define TIMECODECONST_H_

#include <cstdint>

#if defined (GD32)
# include "gd32.h"
# define MASTER_TIMER_CLOCK		(AHB_CLOCK_FREQ)
# define TIMER_PRESCALER		(199)
# define FREQUENCY_EFFECTIVE	(MASTER_TIMER_CLOCK / (TIMER_PRESCALER + 1))
#endif

struct TimeCodeConst {
	static constexpr uint8_t FPS[4] = { 24, 25, 30, 30 };
#if defined (H3)
	static constexpr uint32_t TMR_INTV[4] = {12000000 / 24, 12000000 / 25, 12000000 / 30, 12000000 / 30};
#elif defined (GD32)
	static constexpr uint32_t TMR_INTV[4] = {(FREQUENCY_EFFECTIVE / 24) - 1, (FREQUENCY_EFFECTIVE / 25) - 1, (FREQUENCY_EFFECTIVE / 30) - 1, (FREQUENCY_EFFECTIVE / 30) - 1};
	static_assert((FREQUENCY_EFFECTIVE / 24) <= UINT16_MAX);
#endif
};

#endif /* TIMECODECONST_H_ */
