/**
 * @file ntpserver.h
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

#ifndef NTPSERVER_H_
#define NTPSERVER_H_

#include <stdint.h>
#include <time.h>

#include "ltc.h"

#include "ntp.h"

class NtpServer {
public:
	NtpServer(uint8_t nYear, uint8_t nMonth, uint8_t nDay);
	~NtpServer();

	void Start();
	void Stop();

	void SetTimeCode(const struct TLtcTimeCode *pLtcTimeCode);

	void Run();

	void Print();

	static NtpServer* Get() {
		return s_pThis;
	}

private:
	time_t m_tDate{0};
	time_t m_tTimeDate{0};
	uint32_t m_nFraction{0};
	int32_t m_nHandle{-1};

	struct TNtpPacket m_Request;
	struct TNtpPacket m_Reply;

	static NtpServer *s_pThis;
};

#endif /* NTPSERVER_H_ */
