/**
 * @file platform_ltc.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_PLATFORM_LTC_H_
#define ARM_PLATFORM_LTC_H_

#include <cstdint>

#include "ltc.h"

extern volatile uint32_t gv_ltc_nUpdatesPerSecond;
extern volatile uint32_t gv_ltc_nUpdatesPrevious;
extern volatile uint32_t gv_ltc_nUpdates;

extern volatile bool gv_ltc_bTimeCodeAvailable;
extern volatile uint32_t gv_ltc_nTimeCodeCounter;

extern struct ltc::TimeCode g_ltc_LtcTimeCode;

#if defined (H3)
# define PLATFORM_LTC_ARM
# include "../src/arm/h3/h3_platform_ltc.h"
#elif  defined (GD32)
# define PLATFORM_LTC_ARM
# include "../src/arm/gd32/gd32_platform_ltc.h"
#endif

#endif /* ARM_PLATFORM_LTC_H_ */
