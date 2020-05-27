/**
 * @file ntpclient.h
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

#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

#include <stdint.h>
#include <time.h>

#include "ntp.h"

enum class NtpClientStatus {
	INIT,
	IDLE,
	STOPPED,
	WAITING
};

class NtpClientDisplay {
public:
	virtual ~NtpClientDisplay(void) {
	}

	virtual void ShowNtpClientStatus(NtpClientStatus nStatus)=0;
};

class NtpClient {
public:
	NtpClient(uint32_t nServerIp = 0);
	~NtpClient(void);

	void Init(void);
	void Run(void);

	void Print(void);

	NtpClientStatus GetStatus(void) {
		return m_tStatus;
	}

	void SetNtpClientDisplay(NtpClientDisplay *pNtpClientDisplay) {
		m_pNtpClientDisplay = pNtpClientDisplay;
	}

private:
	void SetUtcOffset(float fUtcOffset);

private:
	uint32_t m_nServerIp;
	int32_t m_nUtcOffset;
	int32_t m_nHandle;
	NtpClientStatus m_tStatus;
	struct TNtpPacket m_Request;
	struct TNtpPacket m_Reply;
	time_t m_InitTime;
	uint32_t m_MillisRequest;
	uint32_t m_MillisLastPoll;

	NtpClientDisplay *m_pNtpClientDisplay = 0;
};

#endif /* NTPCLIENT_H_ */
