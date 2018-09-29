/**
 * @file net_timers.c
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

extern void igmp_timer(void);
#ifndef NDEBUG
 extern void arp_cache_timer(void);
#endif

static volatile uint32_t s_ticker;

#define INTERVAL_US (100*1000)	// 100 msec, 1/10 second

void net_timers_init(void) {
	s_ticker = 0;
}

void net_timers_run(void) {
	const uint32_t micros_now = H3_TIMER->AVS_CNT1;

	if (__builtin_expect((micros_now >= s_ticker), 0)) {
		s_ticker = micros_now + INTERVAL_US;
		igmp_timer();
#ifndef NDEBUG
		arp_cache_timer();
#endif
	}
}
