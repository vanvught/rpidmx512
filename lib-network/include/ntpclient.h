/**
 * @file ntpclient.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

#include <stdint.h>
#include <time.h>

#include "ntp.h"

enum TNtpClientStatus {
	NTP_CLIENT_STATUS_STOPPED,
	NTP_CLIENT_STATUS_IDLE,
	NTP_CLIENT_STATUS_WAITING
};

class NtpClient {
public:
	NtpClient(uint32_t nServerIp = 0);
	~NtpClient(void);

	void Init(void);
	void Run(void);

	void Print(void);

	TNtpClientStatus GetStatus(void) {
		return m_tStatus;
	}

	static NtpClient *Get(void) {
		return s_pThis;
	}

private:
	void SetUtcOffset(float fUtcOffset);

private:
	static NtpClient *s_pThis;

private:
	uint32_t m_nServerIp;
	int32_t m_nUtcOffset;
	int32_t m_nHandle;
	TNtpClientStatus m_tStatus;
	struct TNtpPacket m_Request;
	struct TNtpPacket m_Reply;
	time_t m_InitTime;
	uint32_t m_MillisRequest;
	uint32_t m_MillisLastPoll;
};

#endif /* NTPCLIENT_H_ */
