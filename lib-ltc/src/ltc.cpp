/**
 * @file ltc.cpp
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
#include <cstring>
#include <time.h>
#include <cassert>

#include "ltc.h"

namespace ltc
{
uint32_t g_nDisabledOutputs;
ltc::Type g_Type;

static constexpr char kTypes[5][ltc::timecode::TYPE_MAX_LENGTH + 1] = {"Film 24fps ", "EBU 25fps  ", "DF 29.97fps", "SMPTE 30fps", "----- -----"};

const char* get_type(ltc::Type type)
{
    if (type > ltc::Type::UNKNOWN)
    {
        return kTypes[static_cast<uint32_t>(ltc::Type::UNKNOWN)];
    }

    return kTypes[static_cast<uint32_t>(type)];
}

const char* get_type()
{
    if (ltc::g_Type > ltc::Type::UNKNOWN)
    {
        return kTypes[static_cast<uint32_t>(ltc::Type::UNKNOWN)];
    }

    return kTypes[static_cast<uint32_t>(ltc::g_Type)];
}

ltc::Type get_type(uint8_t fps)
{
    switch (fps)
    {
        case 24:
            return ltc::Type::FILM;
            break;
        case 25:
            return ltc::Type::EBU;
            break;
        case 29:
            return ltc::Type::DF;
            break;
        case 30:
            return ltc::Type::SMPTE;
            break;
        default:
            break;
    }

    return ltc::Type::UNKNOWN;
}

void set_type(uint8_t fps)
{
    ltc::g_Type = get_type(fps);
}

void init_timecode(char* timecode)
{
    assert(timecode != nullptr);

    memset(timecode, ' ', ltc::timecode::CODE_MAX_LENGTH);

    timecode[ltc::timecode::index::COLON_1] = ':';
    timecode[ltc::timecode::index::COLON_2] = ':';
    timecode[ltc::timecode::index::COLON_3] = ':';
}

void init_systemtime(char* systemtime)
{
    assert(systemtime != nullptr);

    memset(systemtime, ' ', ltc::timecode::SYSTIME_MAX_LENGTH);

    systemtime[ltc::systemtime::index::COLON_1] = ':';
    systemtime[ltc::systemtime::index::COLON_2] = ':';
}

static void Itoa(uint32_t value, char* buffer)
{
    auto* p = buffer;

    if (value == 0)
    {
        *p++ = '0';
        *p = '0';
        return;
    }

    *p++ = static_cast<char>('0' + (value / 10U));
    *p = static_cast<char>('0' + (value % 10U));
}

void itoa_base10(const struct ltc::TimeCode* ltc_timecode, char* timecode)
{
    assert(ltc_timecode != nullptr);
    assert(timecode != nullptr);

    Itoa(ltc_timecode->hours, &timecode[ltc::timecode::index::HOURS]);
    Itoa(ltc_timecode->minutes, &timecode[ltc::timecode::index::MINUTES]);
    Itoa(ltc_timecode->seconds, &timecode[ltc::timecode::index::SECONDS]);
    Itoa(ltc_timecode->frames, &timecode[ltc::timecode::index::FRAMES]);
}

void itoa_base10(const struct tm* local_time, char* system_time)
{
    assert(local_time != nullptr);
    assert(system_time != nullptr);

    Itoa(local_time->tm_hour, &system_time[ltc::systemtime::index::HOURS]);
    Itoa(local_time->tm_min, &system_time[ltc::systemtime::index::MINUTES]);
    Itoa(local_time->tm_sec, &system_time[ltc::systemtime::index::SECONDS]);
}

#define DIGIT(x) ((x) - '0')
#define VALUE(x, y) static_cast<uint8_t>(((x) * 10) + (y))

bool parse_timecode(const char* time_code, uint8_t fps, struct ltc::TimeCode* timecode)
{
    assert(time_code != nullptr);
    assert(timecode != nullptr);

    if ((time_code[ltc::timecode::index::COLON_1] != ':') || (time_code[ltc::timecode::index::COLON_2] != ':') ||
        (time_code[ltc::timecode::index::COLON_3] != ':'))
    {
        return false;
    }

    // Frames first

    auto tenths = DIGIT(time_code[ltc::timecode::index::FRAMES_TENS]);

    if ((tenths < 0) || (tenths > 3))
    {
        return false;
    }

    auto digit = DIGIT(time_code[ltc::timecode::index::FRAMES_UNITS]);

    if ((digit < 0) || (digit > 9))
    {
        return false;
    }

    auto value = VALUE(tenths, digit);

    if (value >= fps)
    {
        return false;
    }

    timecode->frames = value;

    // Seconds

    tenths = DIGIT(time_code[ltc::timecode::index::SECONDS_TENS]);

    if ((tenths < 0) || (tenths > 5))
    {
        return false;
    }

    digit = DIGIT(time_code[ltc::timecode::index::SECONDS_UNITS]);
    if ((digit < 0) || (digit > 9))
    {
        return false;
    }

    value = VALUE(tenths, digit);

    if (value > 59)
    {
        return false;
    }

    timecode->seconds = value;

    // Minutes

    tenths = DIGIT(time_code[ltc::timecode::index::MINUTES_TENS]);

    if ((tenths < 0) || (tenths > 5))
    {
        return false;
    }

    digit = DIGIT(time_code[ltc::timecode::index::MINUTES_UNITS]);

    if ((digit < 0) || (digit > 9))
    {
        return false;
    }

    value = VALUE(tenths, digit);

    if (value > 59)
    {
        return false;
    }

    timecode->minutes = value;

    // Hours

    tenths = DIGIT(time_code[ltc::timecode::index::HOURS_TENS]);

    if ((tenths < 0) || (tenths > 2))
    {
        return false;
    }

    digit = DIGIT(time_code[ltc::timecode::index::HOURS_UNITS]);

    if ((digit < 0) || (digit > 9))
    {
        return false;
    }

    value = VALUE(tenths, digit);

    if (value > 23)
    {
        return false;
    }

    timecode->hours = value;

    return true;
}

bool parse_timecode_rate(const char* timecode_rate, uint8_t& fps)
{
    assert(timecode_rate != nullptr);

    const auto kTenths = DIGIT(timecode_rate[0]);

    if ((kTenths < 0) || (kTenths > 3))
    {
        return false;
    }

    const auto kDigit = DIGIT(timecode_rate[1]);

    if ((kDigit < 0) || (kDigit > 9))
    {
        return false;
    }

    const auto kValue = VALUE(kTenths, kDigit);

    switch (kValue)
    {
        case 24:
            fps = 24;
            ltc::g_Type = ltc::Type::FILM;
            break;
        case 25:
            fps = 25;
            ltc::g_Type = ltc::Type::EBU;
            break;
        case 29:
            fps = 30;
            ltc::g_Type = ltc::Type::DF;
            break;
        case 30:
            fps = 30;
            ltc::g_Type = ltc::Type::SMPTE;
            break;
        default:
            return false;
            break;
    }

    return true;
}
} // namespace ltc
