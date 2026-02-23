/**
 * @file gps.cpp
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

/**
 * https://gpsd.gitlab.io/gpsd/NMEA.html
 */

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <cassert>

#include "gps.h"
#include "hal_millis.h"
#include "hal_uart.h"
#include "utc.h"
#include "json/gpsparams.h"
 #include "firmware/debug/debug_debug.h"

// Maximum sentence length, including the $ and <CR><LF> is 82 bytes.

namespace gps::nmea
{
namespace length
{
static constexpr uint32_t kTalkerId = 2;
static constexpr uint32_t kTag = 3;
} // namespace length
enum
{
    kRmc,
    kGga,
    kZda,
    kUndefined
};
} // namespace gps::nmea

constexpr char aTag[static_cast<int>(gps::nmea::kUndefined)][gps::nmea::length::kTag] = {
    {'R', 'M', 'C'}, // Recommended Minimum Navigation Information
    {'G', 'G', 'A'}, // Global Positioning System Fix Data
    {'Z', 'D', 'A'}  // Time & Date - UTC, day, month, year and local time zone
};

GPS* GPS::s_this = nullptr;

GPS::GPS(int32_t utc_offset, gps::Module module) : utc_offset_(hal::utc::IsValidOffset((utc_offset))), module_(module)
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    memset(&tm_, 0, sizeof(struct tm));

    tm_.tm_mday = _TIME_STAMP_DAY_;         // The day of the month, in the range 1 to 31.
    tm_.tm_mon = _TIME_STAMP_MONTH_ - 1;    // The number of months since January, in the range 0 to 11.
    tm_.tm_year = _TIME_STAMP_YEAR_ - 1900; // The number of years since 1900.

    DEBUG_EXIT();
}

uint32_t GPS::GetTag(const char* tag)
{
    for (uint32_t i = 0; i < gps::nmea::kUndefined; i++)
    {
        if (memcmp(aTag[i], tag, gps::nmea::length::kTag) == 0)
        {
            return i;
        }
    }

    return static_cast<uint32_t>(gps::nmea::kUndefined);
}

int32_t GPS::ParseDecimal(const char* p, uint32_t& length)
{
    const auto kIsNegative = (*p == '-');

    length = kIsNegative ? 1 : 0;
    int32_t value = 0;

    while ((p[length] != '.') && (p[length] != ','))
    {
        value = value * 10 + p[length] - '0';
        length++;
    }

    if (p[length] == '.')
    {
        length++;
        value = value * 10 + p[length] - '0';
        length++;
        value = value * 10 + p[length] - '0';
        length++;
    }

    return kIsNegative ? -value : value;
}

void GPS::SetTime(int32_t time)
{
    if (time != 0)
    {
        time_timestamp_millis_ = hal::Millis();
        is_time_updated_ = true;

        time /= 100;
        tm_.tm_sec = time % 100;
        time /= 100;
        tm_.tm_min = time % 100;
        tm_.tm_hour = time / 100;
    }
}

void GPS::SetDate(int32_t date)
{
    if (date != 0)
    {
        date_timestamp_millis_ = hal::Millis();
        is_date_updated_ = true;

        tm_.tm_year = 100 + (date % 100); // The number of years since 1900.
        date /= 100;
        tm_.tm_mon = (date % 100) - 1; // The number of months since January, in the range 0 to 11.
        tm_.tm_mday = date / 100;      // The day of the month, in the range 1 to 31.
    }
}

void GPS::Start()
{
    DEBUG_ENTRY();

    UartInit();

    if (module_ < gps::Module::kUndefined)
    {
        UartSend(gps::kBaud115200[static_cast<uint32_t>(module_)]);
        udelay(100 * 1000);
        UartSetBaud(115200);
        UartSend(gps::kBaud115200[static_cast<uint32_t>(module_)]);

        const auto kMillis = hal::Millis();

        while ((hal::Millis() - kMillis) < 1000)
        {
            sentence_ = const_cast<char*>(UartGetSentence());

            if (sentence_ != nullptr)
            {
                DumpSentence(sentence_);
#ifndef NDEBUG
                printf("[%u]\n", hal::Millis() - kMillis);
#endif
                break;
            }
        }

        if (sentence_ == nullptr)
        {
            UartSetBaud(9600);
        }
    }

    status_current_ = gps::Status::kIdle;

    Display(gps::Status::kIdle);
    DEBUG_EXIT();
}

void GPS::Run()
{
    if (__builtin_expect(((sentence_ = const_cast<char*>(UartGetSentence())) == nullptr), 1))
    {
        return;
    }

    DumpSentence(sentence_);

    uint32_t tag;

    if (__builtin_expect(((tag = GetTag(&sentence_[1 + gps::nmea::length::kTalkerId])) == gps::nmea::kUndefined), 0))
    {
        return;
    }

    DumpSentence(sentence_);

    uint32_t offset = 1 + gps::nmea::length::kTalkerId + gps::nmea::length::kTag + 1; // $ and ,
    uint32_t field_index = 1;

    do
    {
        uint32_t length = 0;
        switch (tag | field_index << 8)
        {
            case gps::nmea::kRmc | (1 << 8): // UTC Time of position, hhmmss.ss
            case gps::nmea::kGga | (1 << 8):
            case gps::nmea::kZda | (1 << 8):
                SetTime(ParseDecimal(&sentence_[offset], length));
                offset += length;
                break;
            case gps::nmea::kRmc | (2 << 8): // Status, A = Valid, V = Warning
                status_current_ = (sentence_[offset] == 'A' ? gps::Status::kValid : gps::Status::kWarning);
                //			printf("(%c) : %d\n", sentence_[nOffset], nOffset	);
                offset += 1;
                break;
            case gps::nmea::kRmc | (9 << 8): // Date, ddmmyy
                SetDate(ParseDecimal(&sentence_[offset], length));
                offset += length;
                break;
            default:
                break;
        }

        field_index++;

        while ((sentence_[offset] != '*') && (sentence_[offset] != ','))
        {
            offset++;
        }

    } while (sentence_[offset++] != '*');

    if (status_current_ != status_previous_)
    {
        status_previous_ = status_current_;

        Display(status_current_);
    }
}

void GPS::DumpSentence([[maybe_unused]] const char* sentence)
{
#ifndef NDEBUG
    printf("%p |", sentence);

    const char* p = sentence;

    while (*p != '\r')
    {
        putchar(*p++);
    }

    puts("|");
#endif
}

void GPS::Print()
{
    printf("GPS [UART%u]\n", EXT_UART_NUMBER);
    printf(" Module : %s [%u]\n", gps::GetModule(module_), baud_);
    printf(" UTC offset : %d (seconds)\n", utc_offset_);
    switch (status_current_)
    {
        case gps::Status::kWarning:
            puts(" No Fix");
            break;
        case gps::Status::kValid:
            puts(" Has Fix");
            break;
        case gps::Status::kIdle:
            puts(" Idle");
            break;
        default:
            break;
    }
}
