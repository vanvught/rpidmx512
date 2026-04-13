/**
 * @file time.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <time.h>

namespace global {
int32_t g_nUtcOffset = 0;
} // namespace global

static constexpr int kDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int Isleapyear(int year) {
    if (year % 100 == 0) {
        return year % 400 == 0;
    }

    return year % 4 == 0;
}

static int Getdaysofmonth(int month, int year) {
    if ((month == 1) && Isleapyear(year)) {
        return 29;
    }

    return kDaysOfMonth[month];
}

static struct tm s_tm;

extern "C" {
struct tm* localtime(const time_t* t) {
    if (t == nullptr) {
        return nullptr;
    }

    auto time = *t + global::g_nUtcOffset;
    return gmtime(&time);
}

struct tm* gmtime(const time_t* t) {
    if (t == nullptr) {
        return nullptr;
    }

    auto time = *t;

    s_tm.tm_sec = time % 60;
    time /= 60;
    s_tm.tm_min = time % 60;
    time /= 60;
    s_tm.tm_hour = time % 24;
    time /= 24;

    s_tm.tm_wday = (time + 4) % 7;

    int year = 1970;

    while (1) {
        const time_t kDaysOfYear = Isleapyear(year) ? 366 : 365;
        if (time < kDaysOfYear) {
            break;
        }

        time -= kDaysOfYear;
        year++;
    }

    s_tm.tm_year = year - 1900;
    s_tm.tm_yday = time;

    int month = 0;

    while (1) {
        const auto kDaysMonth = Getdaysofmonth(month, year);
        if (time < kDaysMonth) {
            break;
        }

        time -= kDaysMonth;
        month++;
    }

    s_tm.tm_mon = month;
    s_tm.tm_mday = time + 1;

    return &s_tm;
}

time_t mktime(struct tm* t) {
    time_t result = 0;

    if (t == nullptr) {
        return -1;
    }

    if (t->tm_year < 70 || t->tm_year > 139) {
        return -1;
    }

    int year;

    for (year = 1970; year < 1900 + t->tm_year; year++) {
        result += Isleapyear(year) ? 366 : 365;
    }

    if (t->tm_mon < 0 || t->tm_mon > 11) {
        return -1;
    }

    int month;

    for (month = 0; month < t->tm_mon; month++) {
        result += Getdaysofmonth(month, t->tm_year);
    }

    if (t->tm_mday < 1 || t->tm_mday > Getdaysofmonth(t->tm_mon, t->tm_year)) {
        return -1;
    }

    result += t->tm_mday - 1;
    result *= 24;

    if (t->tm_hour < 0 || t->tm_hour > 23) {
        return -1;
    }

    result += t->tm_hour;
    result *= 60;

    if (t->tm_min < 0 || t->tm_min > 59) {
        return -1;
    }

    result += t->tm_min;
    result *= 60;

    if (t->tm_sec < 0 || t->tm_sec > 59) {
        return -1;
    }

    result += t->tm_sec;

    return result;
}
}
