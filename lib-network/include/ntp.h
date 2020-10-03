/**
 * @file ntp.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#define LOCAL_TIME_YEAR_OFFSET	1900

#define NTP_TIMESTAMP_DELTA 2208988800

enum TNtpUdpPort {
	NTP_UDP_PORT = 123
};

enum TNtpVersion {
	NTP_VERSION = (4 << 3)
};

enum TNtpMode {
	NTP_MODE_CLIENT = (3 << 0),
	NTP_MODE_SERVER = (4 << 0)
};

enum TNtpStratum {
	NTP_STRATUM = 2
};

enum TNtpPoll {
	NTP_MINPOLL = 4
};

struct TNtpPacket {
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

#endif /* NTP_H_ */
