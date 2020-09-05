/**
 * @file time.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
 * PoC Code -> Do not use in production
 */

#include <stddef.h>
#include <sys/time.h>
#include <assert.h>

#include <stdio.h>

#include "h3.h"

#include "debug.h"

static uint32_t set_hs_timer = 0;
static uint64_t s_micros = 0;

#define MICROS_SECONDS	1000000

/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, __attribute__((unused))  struct timezone *tz) {
	assert(tv != 0);

	const uint32_t hs_timer = H3_HS_TIMER->CURNT_LO / 100;

	uint32_t hs_timer_elapsed;

	if (set_hs_timer >= hs_timer) {
		hs_timer_elapsed = set_hs_timer - hs_timer;
	} else {
		hs_timer_elapsed = (uint32_t)(~0) / 100 - (hs_timer - set_hs_timer);
	}

	set_hs_timer = hs_timer;
	s_micros += hs_timer_elapsed;

	tv->tv_sec = s_micros / MICROS_SECONDS;
	tv->tv_usec = (suseconds_t) (s_micros - ((uint64_t) tv->tv_sec * MICROS_SECONDS));

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused)) const struct timezone *tz) {
	assert(tv != 0);

	set_hs_timer = H3_HS_TIMER->CURNT_LO / 100;
	s_micros = ((uint64_t) tv->tv_sec * MICROS_SECONDS) + (uint64_t) tv->tv_usec;

	return 0;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	if (__timer != NULL) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}
