/**
 * @file hwclockrun.cpp
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <sys/time.h>
#include <time.h>

#include "hwclock.h"
#include "hal_millis.h"
 #include "firmware/debug/debug_debug.h"

enum class Status
{
    kWaiting,
    kSampling
};

static Status s_status = Status::kWaiting;
static time_t s_seconds;
static int32_t s_seconds_t1;
static struct tm s_rtc_t1;
static struct timeval s_tv_t1;
static struct tm s_rtc_t2;
static struct timeval s_tv_t2;

void HwClock::Process()
{
    if (s_status == Status::kWaiting)
    {
        if (__builtin_expect(((hal::Millis() - last_hc_to_sys_millis_) > 7200 * 1000), 0))
        {
            s_status = Status::kSampling;

            RtcGet(&s_rtc_t1);
            gettimeofday(&s_tv_t1, nullptr);

            s_seconds_t1 = s_rtc_t1.tm_sec + s_rtc_t1.tm_min * 60;
            s_seconds = mktime(&s_rtc_t1);
        }

        return;
    }

    if (s_status == Status::kSampling)
    {
        RtcGet(&s_rtc_t2);

        const auto kSeconds2 = s_rtc_t2.tm_sec + s_rtc_t2.tm_min * 60;

        if (s_seconds_t1 != kSeconds2)
        {
            gettimeofday(&s_tv_t2, nullptr);

            struct timeval tv;
            tv.tv_sec = s_seconds;

            if (s_tv_t2.tv_sec == s_tv_t1.tv_sec)
            {
                tv.tv_usec = 1000000 - (s_tv_t2.tv_usec - s_tv_t1.tv_usec);
            }
            else
            {
                if (s_tv_t2.tv_usec - s_tv_t1.tv_usec >= 0)
                {
                    tv.tv_usec = s_tv_t2.tv_usec - s_tv_t1.tv_usec;
                }
                else
                {
                    tv.tv_usec = s_tv_t1.tv_usec - s_tv_t2.tv_usec;
                }
            }

            settimeofday(&tv, nullptr);

            last_hc_to_sys_millis_ = hal::Millis();
            s_status = Status::kWaiting;

            DEBUG_PRINTF("%d:%d (%d %d) (%d %d) -> %d", s_seconds_t1, kSeconds2, static_cast<int>(s_tv_t1.tv_sec), static_cast<int>(s_tv_t1.tv_usec),
                         static_cast<int>(s_tv_t2.tv_sec), static_cast<int>(s_tv_t2.tv_usec), static_cast<int>(tv.tv_usec));
        }

        return;
    }
}
