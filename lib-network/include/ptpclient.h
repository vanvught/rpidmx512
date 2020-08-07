/**
 * @file ptpclient.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PTPCLIENT_H_
#define PTPCLIENT_H_

#include <stdint.h>
#include <sys/time.h>

#include "ptp.h"

class PtpClientDisplay {
public:
	virtual ~PtpClientDisplay() {
	}
};

class PtpClient {
public:
	PtpClient();

	void Start();
	void Stop();
	void Run();

	void Print();

	void SetDisplay(PtpClientDisplay *pPtpClientDisplay) {
		m_pPtpClientDisplay = pPtpClientDisplay;
	}

private:
	void HandleEventMessage();
	void HandleGeneralMessage();
	void GetTime() {
        struct timeval tv = {0, 0};
        gettimeofday(&tv, nullptr);
        m_nSeconds =  static_cast<uint32_t>(tv.tv_sec);
        m_nMicroSeconds =  static_cast<uint32_t>(tv.tv_usec) * 1000U;
	}

private:
	int32_t m_nHandleEvent{-1};
	int32_t m_nHandleGeneral{-1};
	uint32_t m_nMulticastIp{0};

	PTPMessage m_Message;
	uint32_t m_nBytesReceived;
	uint32_t m_nMasterIpAddress;

	uint32_t m_nSeconds;
	uint32_t m_nMicroSeconds;
	uint16_t m_DelayReqSequenceId{0};

	PtpClientDisplay *m_pPtpClientDisplay{nullptr};

	struct TimeStamp {
		uint32_t Seconds;
		uint32_t NanoSeconds;
	};

	TimeStamp T1{0, 0};
	TimeStamp T2{0, 0};
	TimeStamp T3{0, 0};
	TimeStamp T4{0, 0};

	struct PTPDelayReq m_DelayReq;
};

#endif /* PTPCLIENT_H_ */
