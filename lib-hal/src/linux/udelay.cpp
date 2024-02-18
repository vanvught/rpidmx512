/**
 * @file udelay.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <time.h>
#include <sys/time.h>

static constexpr uint32_t TICKS_PER_US = 1;

static uint32_t micros() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (uint32_t)((tv.tv_sec * 1000000) + tv.tv_usec);
}

void udelay(uint32_t nMicros, uint32_t nOffsetMicros) {
	const auto nTicks = nMicros * TICKS_PER_US;

	uint32_t nTicksCount = 0;
	uint32_t nTicksPrevious;

	if (nOffsetMicros == 0) {
		nTicksPrevious = micros();
	} else {
		nTicksPrevious = nOffsetMicros;
	}

	while (1) {
		const auto nTicksNow = micros();

		if (nTicksNow != nTicksPrevious) {
			if (nTicksNow > nTicksPrevious) {
				nTicksCount += nTicksNow - nTicksPrevious;
			} else {
				nTicksCount += UINT32_MAX - nTicksPrevious + nTicksNow;
			}

			if (nTicksCount >= nTicks) {
				break;
			}

			nTicksPrevious = nTicksNow;
		}
	}
}
