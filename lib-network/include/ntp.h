/**
 * @file ntp.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NTP_H_
#define NTP_H_

#include <cstdint>

namespace ntp {
static constexpr uint32_t LOCAL_TIME_YEAR_OFFSET = 1900;
static constexpr uint32_t NTP_TIMESTAMP_DELTA = 2208988800u;
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
}  // namespace ntp

#endif /* NTP_H_ */
