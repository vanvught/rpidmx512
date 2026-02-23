/**
 * @file asctime.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <ctime>

static constexpr uint32_t kMaxAscTime = 50;

static const char kMonName[][4]  = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char kWdayName[][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static char s_buffer[kMaxAscTime + 1];

extern "C" char* asctime(const struct tm* p_tm)
{
    if (!p_tm) return nullptr;

    const char* const kWday = (p_tm->tm_wday >= 0 && p_tm->tm_wday <= 6)  ? kWdayName[p_tm->tm_wday] : "???";
    const char* const kMon  = (p_tm->tm_mon  >= 0 && p_tm->tm_mon  <= 11) ? kMonName[p_tm->tm_mon]   : "???";

    snprintf(s_buffer, sizeof s_buffer,
                  "%s %s %2d %02d:%02d:%02d %04d",
                  kWday, kMon,
                  p_tm->tm_mday,
                  p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
                  p_tm->tm_year + 1900);

    return s_buffer;
}

