/**
 * @file gpstimeclient.cpp
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

#include "gpstimeclient.h"
#include "hal_millis.h"
#include "platform_gpio.h"
 #include "firmware/debug/debug_debug.h"

GPSTimeClient::GPSTimeClient(int32_t utc_offset, gps::Module module) : GPS(utc_offset, module), wait_pps_millis_(hal::Millis())
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void GPSTimeClient::Start()
{
    DEBUG_ENTRY();

    GPS::Start();

    platform_gpio_init();

    DEBUG_EXIT();
}

void GPSTimeClient::Run()
{
    GPS::Run();

    if (GPS::GetStatus() == gps::Status::kIdle)
    {
        return;
    }

    if (platform_is_pps())
    {
        struct timeval tv;
        tv.tv_sec = GetLocalSeconds() + 1;
        tv.tv_usec = 0;
        settimeofday(&tv, nullptr);

        wait_pps_millis_ = hal::Millis();

        DEBUG_PUTS("PPS handled");
        return;
    }

    const auto kMillis = hal::Millis();

    if (__builtin_expect(((kMillis - wait_pps_millis_) > (10 * 1000)), 0))
    {
        wait_pps_millis_ = kMillis;
        // There is no PPS
        if (GPS::IsTimeUpdated())
        {
            const auto kElapsedMillis = kMillis - GetTimeTimestampMillis();

            if (kElapsedMillis <= 1)
            {
                struct timeval tv;
                tv.tv_sec = GPS::GetLocalSeconds();
                tv.tv_usec = 0;
                settimeofday(&tv, nullptr);

                DEBUG_PRINTF("(GPS::IsTimeUpdated()) %u", kElapsedMillis);
            }
        }

        return;
    }

    return;
}
