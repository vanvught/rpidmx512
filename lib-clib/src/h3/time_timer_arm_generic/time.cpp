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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstdio>
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
	assert(tv != nullptr);

	const uint64_t current_cntvct = read_cntpct();
	const uint64_t elapsed_ticks = current_cntvct - set_timer; // No roll-over issues with 64-bit arithmetic

	// Convert ticks to microseconds with fractional correction
	static uint64_t fractional_ticks = 0; // Persistent fractional part
	const uint64_t total_usec = elapsed_ticks * 1000000ULL + fractional_ticks; // Scaled to microseconds
	const uint64_t elapsed_usec = total_usec / 24000000ULL; // Correctly divide by clock frequency
	fractional_ticks = total_usec % 24000000ULL; // Keep the fractional remainder

	const time_t elapsed_sec = elapsed_usec / 1000000ULL;
	const suseconds_t elapsed_subsec = elapsed_usec % 1000000ULL;

	// Compute the current time
	tv->tv_sec = s_tv.tv_sec + elapsed_sec;
	tv->tv_usec = s_tv.tv_usec + elapsed_subsec;

	// Handle microsecond overflow
	if (tv->tv_usec >= 1000000) {
		tv->tv_sec++;
		tv->tv_usec -= 1000000;
	} else if (tv->tv_usec < 0) {
		tv->tv_sec--;
		tv->tv_usec += 1000000;
	}

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused)) const struct timezone *tz) {
    assert(tv != nullptr);

    // Capture the current counter value as the reference point
    const uint64_t current_cntvct = read_cntpct();

    // Calculate the elapsed microseconds since the last set_timer
    const uint64_t elapsed_ticks = current_cntvct - set_timer;
    static uint64_t fractional_ticks = 0;
    const uint64_t total_usec = elapsed_ticks * 1000000ULL + fractional_ticks;
    const uint64_t elapsed_usec = total_usec / 24000000ULL;
    fractional_ticks = total_usec % 24000000ULL;

    // Adjust s_tv based on the elapsed time
    s_tv.tv_sec += elapsed_usec / 1000000ULL;
    s_tv.tv_usec += elapsed_usec % 1000000ULL;

    // Normalize s_tv to ensure tv_usec is valid
    if (s_tv.tv_usec >= 1000000) {
        s_tv.tv_sec++;
        s_tv.tv_usec -= 1000000;
    } else if (s_tv.tv_usec < 0) {
        s_tv.tv_sec--;
        s_tv.tv_usec += 1000000;
    }

    // Set the new time
    s_tv.tv_sec = tv->tv_sec;
    s_tv.tv_usec = tv->tv_usec;

    // Update the reference timer
    set_timer = current_cntvct;

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
