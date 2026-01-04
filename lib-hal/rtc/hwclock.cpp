/**
 * @file hwclock.cpp
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

#if defined(DEBUG_HWCLOCK)
#undef NDEBUG
#endif

#include <cassert>
#include <cstdio>
#include <sys/time.h>

#include "hwclock.h"

#include "hal_watchdog.h"
#include "hal_millis.h"

 #include "firmware/debug/debug_debug.h"

HwClock::HwClock()
{
    assert(s_this == nullptr);
    s_this = this;
}

void HwClock::Print()
{
    if (!is_connected_)
    {
        puts("No RTC connected");
        return;
    }

    const char* type = "Unknown";

    switch (type_)
    {
        case rtc::Type::kMcP7941X:
            type = "MCP7941X";
            break;
        case rtc::Type::kDS3231:
            type = "DS3231";
            break;
        case rtc::Type::kPcF8563:
            type = "PCF8563";
            break;
        case rtc::Type::kSocInternal:
            type = "SOC_INTERNAL";
            break;
        default:
            break;
    }

    struct tm tm;
    RtcGet(&tm);
    printf("%s %.4d/%.2d/%.2d %.2d:%.2d:%.2d\n", type, 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/*
 * Set the System Clock from the Hardware Clock.
 */
void HwClock::HcToSys()
{
    DEBUG_ENTRY();
    if (!is_connected_)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kIsWatchdog = hal::Watchdog();

    if (kIsWatchdog)
    {
        hal::WatchdogStop();
    }

    struct tm rtc_t1;
    struct timeval tv_t1;

    RtcGet(&rtc_t1);
    gettimeofday(&tv_t1, nullptr);

    const auto kSecondsT1 = rtc_t1.tm_sec + rtc_t1.tm_min * 60;
    const auto kSeconds = mktime(&rtc_t1);

    struct tm rtc_t2;
    struct timeval tv_t2;

    while (true)
    {
        RtcGet(&rtc_t2);

        const auto kSeconds2 = rtc_t2.tm_sec + rtc_t2.tm_min * 60;

        if (kSecondsT1 != kSeconds2)
        {
            gettimeofday(&tv_t2, nullptr);
            break;
        }
    }

    struct timeval tv;
    tv.tv_sec = kSeconds;

    if (tv_t2.tv_sec == tv_t1.tv_sec)
    {
        tv.tv_usec = 1000000 - (tv_t2.tv_usec - tv_t1.tv_usec);
    }
    else
    {
        if (tv_t2.tv_usec - tv_t1.tv_usec >= 0)
        {
            tv.tv_usec = tv_t2.tv_usec - tv_t1.tv_usec;
        }
        else
        {
            tv.tv_usec = tv_t1.tv_usec - tv_t2.tv_usec;
        }
    }

    settimeofday(&tv, nullptr);

    last_hc_to_sys_millis_ = hal::Millis();

    if (kIsWatchdog)
    {
        hal::WatchdogInit();
    }

    DEBUG_EXIT();
}

/*
 * Set the Hardware Clock from the System Clock.
 */
void HwClock::SysToHc()
{
    DEBUG_ENTRY();
    if (!is_connected_)
    {
        DEBUG_EXIT();
        return;
    }

    const auto kIsWatchdog = hal::Watchdog();

    if (kIsWatchdog)
    {
        hal::WatchdogStop();
    }

    struct timeval tv1;
    gettimeofday(&tv1, nullptr);

    while (true)
    {
        struct timeval tv2;
        gettimeofday(&tv2, nullptr);

        if (tv2.tv_sec >= (tv1.tv_sec + 1))
        {
            const auto* tm = gmtime(&tv2.tv_sec);
            RtcSet(tm);
            break;
        }
    }

    if (kIsWatchdog)
    {
        hal::WatchdogInit();
    }

    DEBUG_EXIT();
}
