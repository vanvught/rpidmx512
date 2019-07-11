/**
 * @file ntpserver.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NTPSERVER_H_
#define NTPSERVER_H_

#include <stdint.h>
#include <time.h>

#include "ltc.h"

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

class NtpServer {
public:
	NtpServer(uint8_t nYear, uint8_t nMonth, uint8_t nDay);
	~NtpServer(void);

	void Start(void);
	void Stop(void);

	void SetTimeCode(const struct TLtcTimeCode *pLtcTimeCode);

	void Run(void);

	void Print(void);

	static NtpServer* Get(void) {
		return s_pThis;
	}

private:
	static NtpServer *s_pThis;

private:
	time_t m_tDate;
	time_t m_tTimeDate;
	uint32_t m_nFraction;
	int32_t m_nHandle;

	struct TNtpPacket m_Request;
	struct TNtpPacket m_Reply;
};

#endif /* NTPSERVER_H_ */
