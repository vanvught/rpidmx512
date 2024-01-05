/**
 * @file hwclockset.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>
#include <time.h>
#include <sys/time.h>

#include "hwclock.h"

#include "debug.h"

bool HwClock::Set(const tm *pTime) {
	if (!m_bIsConnected) {
		return false;
	}

	struct timeval tvT1;
	gettimeofday(&tvT1, nullptr);

	RtcSet(pTime);

	struct timeval tv;
	tv.tv_sec = mktime(const_cast<tm *>(pTime));

	struct timeval tvT2;
	gettimeofday(&tvT2, nullptr);

	if (tvT2.tv_usec - tvT1.tv_usec >= 0) {
		tv.tv_usec = tvT2.tv_usec - tvT1.tv_usec;
	} else {
		tv.tv_usec = 1000000 - (tvT1.tv_usec - tvT2.tv_usec);
	}

	settimeofday(&tv, nullptr);

	return true;
}
