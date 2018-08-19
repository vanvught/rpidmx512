/**
 * @file udelay.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "h3.h"
#include "h3_hs_timer.h"

#define _USE_HS_TIMER_UDELAY

void udelay(uint32_t d) {
#if defined (_USE_AVS_TIMER_UDELAY)
	uint32_t t1 = H3_TIMER->AVS_CNT1;
	const uint32_t t2 = t1 + d;
	do {
		t1 = H3_TIMER->AVS_CNT1;
	}while (t2 >= t1);
#elif defined (_USE_HS_TIMER_UDELAY)
	h3_hs_timer_delay(100 * d);
#else
	uint64_t cval;
	asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));

	uint32_t t1 = (uint32_t) (cval & 0xFFFFFFFF) / 24;
	const uint32_t t2 = t1 + d;

	do {
		asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
		t1 = (uint32_t) (cval & 0xFFFFFFFF) / 24;
	} while (t1 < t2);
#endif
}
