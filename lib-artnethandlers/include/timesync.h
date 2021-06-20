/**
 * @file timesync.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TIMESYNC_H_
#define TIMESYNC_H_

#include <cstdint>

#include "artnettimesync.h"

#include "hardware.h"

class TimeSync: public ArtNetTimeSync {
public:
	TimeSync() {}

	void Handler(const struct TArtNetTimeSync *pArtNetTimeSync) override {
		struct tm tmTime;

		tmTime.tm_sec = pArtNetTimeSync->tm_sec;
		tmTime.tm_min = pArtNetTimeSync->tm_min;
		tmTime.tm_hour = pArtNetTimeSync->tm_hour;
		tmTime.tm_mday = pArtNetTimeSync->tm_mday;
		tmTime.tm_mon = pArtNetTimeSync->tm_mon;
		tmTime.tm_year = ((pArtNetTimeSync->tm_year_hi) << 8) + pArtNetTimeSync->tm_year_lo;

		Hardware::Get()->SetTime(&tmTime);
	}
};

#endif /* TIMESYNC_H_ */
