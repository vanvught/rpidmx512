/**
 * @file ntp.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PROTOCOL_NTP_H_
#define NET_PROTOCOL_NTP_H_

#include <cstdint>

namespace ntp {
static constexpr uint32_t JAN_1970 = 0x83aa7e80; 	// 2208988800 1970 - 1900 in seconds
static constexpr uint32_t LOCAL_TIME_YEAR_OFFSET = 1900;
static constexpr uint32_t MICROSECONDS_IN_SECOND = 1000000;
static constexpr uint16_t UDP_PORT = 123;
static constexpr uint8_t  VERSION = (4U << 3);
static constexpr uint8_t  MODE_CLIENT = (3U << 0);
static constexpr uint8_t  MODE_SERVER = (4U << 0);
static constexpr uint8_t  STRATUM = 2;
static constexpr uint8_t  MINPOLL = 4;

struct Packet {
	uint8_t LiVnMode;
	uint8_t Stratum;
	uint8_t Poll;
	uint8_t Precision;
	uint32_t RootDelay;
	uint32_t RootDispersion;
	uint32_t ReferenceID;
	uint32_t ReferenceTimestamp_s;
	uint32_t ReferenceTimestamp_f;
	uint32_t OriginTimestamp_s;
	uint32_t OriginTimestamp_f;
	uint32_t ReceiveTimestamp_s;
	uint32_t ReceiveTimestamp_f;
	uint32_t TransmitTimestamp_s;
	uint32_t TransmitTimestamp_f;
}__attribute__((packed));

struct TimeStamp {
	uint32_t nSeconds;
	uint32_t nFraction;
};

struct time_t {
	int32_t tv_sec;
	int32_t tv_usec;
};

enum class Status {
	STOPPED, IDLE, WAITING, FAILED
};

static constexpr char STATUS[4][8] = { "Stopped", "Idle", "Waiting", "Failed" };

enum class Modes {
	BASIC, INTERLEAVED, UNKNOWN
};

inline void normalize_time(ntp::time_t *r) {
	r->tv_sec += r->tv_usec / 1000000;
	r->tv_usec -= r->tv_usec / 1000000 * 1000000;

	if (r->tv_sec > 0 && r->tv_usec < 0) {
		r->tv_sec -= 1;
		r->tv_usec += 1000000;
	} else if (r->tv_sec < 0 && r->tv_usec > 0) {
		r->tv_sec += 1;
		r->tv_usec -= 1000000;
	}
}

inline void sub_time(struct ntp::time_t *r, const struct ntp::time_t *x, const struct ntp::time_t *y) {
    r->tv_sec = x->tv_sec - y->tv_sec;
    r->tv_usec = x->tv_usec - y->tv_usec;

    normalize_time(r);
}
}  // namespace ntp

#endif /* NET_PROTOCOL_NTP_H_ */
