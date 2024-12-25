/**
 * @file time.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "h3.h"

static uint64_t set_timer = 0;
static struct timeval s_tv;

static inline uint64_t read_cntpct() {
    uint32_t lo, hi;
    asm volatile ("mrrc p15, 0, %0, %1, c14" : "=r" (lo), "=r" (hi));
    return ((uint64_t)hi << 32) | lo;
}

extern "C" {
int gettimeofday(struct timeval *tv, __attribute__((unused)) struct timezone *tz) {
    assert(tv != nullpt);

    const uint64_t current_cntvct = read_cntpct();
    const uint64_t elapsed_ticks = current_cntvct - set_timer; // No roll-over issues with 64-bit arithmetic

    // Convert ticks to microseconds
    const uint64_t elapsed_usec = elapsed_ticks / 24U;	// Adjust for 24MHz clock H3_F_24M;
    const time_t elapsed_sec = elapsed_usec / 1000000ULL ;
    const suseconds_t elapsed_subsec = elapsed_usec % 1000000ULL;

    // Compute the current time
    tv->tv_sec = s_tv.tv_sec + elapsed_sec;
    tv->tv_usec = s_tv.tv_usec + elapsed_subsec;

    // Handle microsecond overflow
    if (tv->tv_usec >= 1000000) {
        tv->tv_sec++;
        tv->tv_usec -= 1000000;
    }

    return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused)) const struct timezone *tz) {
    assert(tv != nullptr);

    // Capture the current virtual counter value as the reference
    set_timer = read_cntpct();

    // Set the new time
    s_tv.tv_sec = tv->tv_sec;
    s_tv.tv_usec = tv->tv_usec;

    return 0;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	if (__timer != nullptr) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}

}
