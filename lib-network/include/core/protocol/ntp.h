/**
 * @file ntp.h
 *
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_PROTOCOL_NTP_H_
#define CORE_PROTOCOL_NTP_H_

#include <cstdint>

namespace ntp
{
inline constexpr uint32_t kJan1970 = 0x83aa7e80; // 2208988800 1970 - 1900 in seconds
inline constexpr uint32_t kLocalTimeYearOffset = 1900;
inline constexpr uint32_t kMicrosecondsInSecond = 1000000;
inline constexpr uint8_t kVersion = (4U << 3);
inline constexpr uint8_t kModeClient = (3U << 0);
inline constexpr uint8_t kModeServer = (4U << 0);
inline constexpr uint8_t kStratum = 2;
inline constexpr uint8_t kMinpoll = 4;

struct Packet
{
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_id;
    uint32_t reference_timestamp_s;
    uint32_t reference_timestamp_f;
    uint32_t origin_timestamp_s;
    uint32_t origin_timestamp_f;
    uint32_t receive_timestamp_s;
    uint32_t receive_timestamp_f;
    uint32_t transmit_timestamp_s;
    uint32_t transmit_timestamp_f;
} __attribute__((packed));

struct TimeStamp
{
    uint32_t seconds;
    uint32_t fraction;
};

struct Time
{
    int32_t tv_sec;
    int32_t tv_usec;
};

enum class Status
{
    kStopped,
    kIdle,
    kWaiting,
    kLocked,
    kFailed,
    kDisabled
};

inline constexpr char kStatus[][9] = {"Stopped", "Idle", "Waiting", "Locked", "Failed", "Disabled"};

enum class Modes
{
    kBasic,
    kInterleaved,
    kUnknown
};

inline void NormalizeTime(ntp::Time* r)
{
    r->tv_sec += r->tv_usec / 1000000;
    r->tv_usec -= r->tv_usec / 1000000 * 1000000;

    if (r->tv_sec > 0 && r->tv_usec < 0)
    {
        r->tv_sec -= 1;
        r->tv_usec += 1000000;
    }
    else if (r->tv_sec < 0 && r->tv_usec > 0)
    {
        r->tv_sec += 1;
        r->tv_usec -= 1000000;
    }
}

inline void SubTime(struct ntp::Time* r, const struct ntp::Time* x, const struct ntp::Time* y)
{
    r->tv_sec = x->tv_sec - y->tv_sec;
    r->tv_usec = x->tv_usec - y->tv_usec;

    NormalizeTime(r);
}
} // namespace ntp

#endif /* CORE_PROTOCOL_NTP_H_ */
