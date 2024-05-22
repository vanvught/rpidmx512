/**
 * @file ntpserver.h
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

#ifndef NTPSERVER_H_
#define NTPSERVER_H_

#include <cstdint>
#include <time.h>

#include "ltc.h"
#include "ntp.h"
#include "network.h"

#include "debug.h"

class NtpServer {
public:
	NtpServer(const uint32_t nYear, const uint32_t nMonth, const uint32_t nDay);
	~NtpServer();

	void Start();
	void Stop();

	void SetTimeCode(const struct ltc::TimeCode *pLtcTimeCode);

	void Run() {
		uint32_t nIPAddressFrom;
		uint16_t nForeignPort;
		ntp::Packet *pRequest;

		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&pRequest)), &nIPAddressFrom, &nForeignPort);

		if (__builtin_expect((nBytesReceived < sizeof(struct ntp::Packet)), 1)) {
			DEBUG_PUTS("");
			return;
		}

		if (__builtin_expect(((pRequest->LiVnMode & ntp::MODE_CLIENT) != ntp::MODE_CLIENT), 0)) {
			DEBUG_PUTS("");
			return;
		}

		s_Reply.OriginTimestamp_s = pRequest->TransmitTimestamp_s;
		s_Reply.OriginTimestamp_f = pRequest->TransmitTimestamp_f;

		Network::Get()->SendTo(m_nHandle, &s_Reply, sizeof(struct ntp::Packet), nIPAddressFrom, nForeignPort);
	}

	void Print();

	static NtpServer* Get() {
		return s_pThis;
	}

private:
	time_t m_Time { 0 };
	time_t m_tTimeDate { 0 };
	uint32_t m_nFraction { 0 };
	int32_t m_nHandle { -1 };

	static ntp::Packet s_Reply;
	static NtpServer *s_pThis;
};

#endif /* NTPSERVER_H_ */
