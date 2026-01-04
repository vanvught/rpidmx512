/**
 * @file json_datetime.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cstdint>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "global.h"
#include "json/json_key.h"
#include "json/json_parsehelper.h"
#include "json/json_parser.h"
#include "utc.h"
#include "configstore.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

namespace json
{
uint32_t GetTimeofday(char* out_buffer, uint32_t out_buffer_size)
{
    DEBUG_ENTRY();

    struct timeval tv;
    if (gettimeofday(&tv, nullptr) >= 0)
    {
        auto* tm = localtime(&tv.tv_sec);

        int32_t hours;
        uint32_t minutes;
        Global::Instance().GetUtcOffset(hours, minutes);

        if ((hours == 0) && (minutes == 0))
        {
            const auto kLength = static_cast<uint32_t>(snprintf(out_buffer, out_buffer_size, "{\"date\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2dZ\"}\n", 1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec));

            DEBUG_EXIT();
            return kLength;
        }
        else
        {
            const auto kLength = static_cast<uint32_t>(
                snprintf(out_buffer, out_buffer_size, "{\"date\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2d%s%.2d:%.2u\"}\n", 1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, hours > 0 ? "+" : "", hours, minutes));

            DEBUG_EXIT();
            return kLength;
        }
    }

    DEBUG_EXIT();
    return 0;
}

static void SetDate(const char* date, uint32_t date_length)
{
    if ((date_length == 20) || (date_length == 25))
    {
        struct tm tm;
        tm.tm_year = json::Atoi(&date[0], 4) - 1900;
        tm.tm_mon = json::Atoi(&date[5], 2) - 1;
        tm.tm_mday = json::Atoi(&date[8], 2);
        tm.tm_hour = json::Atoi(&date[11], 2);
        tm.tm_min = json::Atoi(&date[12], 2);
        tm.tm_sec = json::Atoi(&date[17], 2);

        struct timeval tv;
        tv.tv_sec = mktime(&tm);
        tv.tv_usec = 0;

        if (date_length == 20)
        {
            assert(date[19] == 'Z');
        }
        else
        {
            const int32_t kSign = date[19] == '-' ? -1 : 1;
            const auto kHours = static_cast<int8_t>(Atoi(&date[20], 2) * kSign);
            const auto kMinutes = static_cast<uint8_t>(Atoi(&date[23], 2));
            int32_t utc_offset;

            if (hal::utc::ValidateOffset(kHours, kMinutes, utc_offset))
            {
                ConfigStore::Instance().GlobalUpdate(&common::store::Global::utc_offset, utc_offset);
                Global::Instance().SetUtcOffsetIfValid(kHours, kMinutes);
            }

            tv.tv_sec = tv.tv_sec - Global::Instance().GetUtcOffset();
        }

        settimeofday(&tv, nullptr);

        DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        DEBUG_EXIT();
        return;
    }
}	

static constexpr json::SimpleKey kDate {
    "date",
    4,
    Fnv1a32("date", 4)
};

static constexpr json::Key kActionKeys[] = {
	json::MakeKey(SetDate, kDate)
};

void SetTimeofday(const char* buffer, uint32_t buffer_size)
{
    DEBUG_ENTRY();
    debug::Dump(buffer, buffer_size);

	ParseJsonWithTable(buffer, buffer_size, kActionKeys);

    DEBUG_EXIT();
}
} // namespace json