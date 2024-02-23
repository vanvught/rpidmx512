/**
 * @file ntpclient.h
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

#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

#include <cstdint>
#include <cstdio>
#include <time.h>

#include "ntp.h"
#include "hardware.h"

#include "debug.h"

namespace ntpclient {
static constexpr uint32_t TIMEOUT_MILLIS = 3000; 	// 3 seconds
static constexpr uint32_t POLL_SECONDS = 1024;		// 2ˆ10

enum class Status {
	STOPPED, IDLE, WAITING, FAILED
};

void display_status(const Status status);
}  // namespace ntpclient

class NtpClient {
public:
	NtpClient(const uint32_t nServerIp = 0);

	void Start();
	void Stop();
	void Print();

	ntpclient::Status GetStatus() const {
		return m_Status;
	}

	void Run() {
		if (m_Status == ntpclient::Status::STOPPED) {
			return;
		}

		if ((m_Status == ntpclient::Status::IDLE) || (m_Status == ntpclient::Status::FAILED)) {
			if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisLastPoll) > (1000 * ntpclient::POLL_SECONDS)), 0)) {
				Send();
				m_MillisRequest = Hardware::Get()->Millis();
				m_Status = ntpclient::Status::WAITING;
				ntpclient::display_status(ntpclient::Status::WAITING);
				DEBUG_PUTS("ntpclient::Status::WAITING");
			}

			return;
		}

		if (m_Status == ntpclient::Status::WAITING) {
			uint8_t LiVnMode;

			if (!Receive(LiVnMode)) {
				if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisRequest) > ntpclient::TIMEOUT_MILLIS), 0)) {
					m_Status = ntpclient::Status::FAILED;
					ntpclient::display_status(ntpclient::Status::FAILED);
					DEBUG_PUTS("ntpclient::Status::FAILED");
				}

				return;
			}

			if (__builtin_expect(((LiVnMode & ntp::MODE_SERVER) == ntp::MODE_SERVER), 1)) {
				m_MillisLastPoll = Hardware::Get()->Millis();

				if (SetTimeOfDay() == 0) {
#ifndef NDEBUG
					const auto nTime = time(nullptr);
					const auto *pLocalTime = localtime(&nTime);
					DEBUG_PRINTF("localtime: %.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
				} else {
					// Error
				}
			} else {
				DEBUG_PUTS("!>> Invalid reply <<!");
			}

			m_Status = ntpclient::Status::IDLE;
			ntpclient::display_status(ntpclient::Status::IDLE);
			DEBUG_PUTS("ntpclient::Status::IDLE");
		}
	}

	static NtpClient *Get() {
		return s_pThis;
	}

private:
	void GetTimeNtpFormat(uint32_t &nSeconds, uint32_t &nFraction);
	void Send();
	bool Receive(uint8_t& LiVnMode);

	struct TimeStamp {
		uint32_t nSeconds;
		uint32_t nFraction;
	};

	void Difference(const struct TimeStamp *Start, const struct TimeStamp *Stop, int32_t &nDiffSeconds, uint32_t &nDiffFraction);
	int SetTimeOfDay();

	void PrintNtpTime(const char *pText, const struct TimeStamp *pNtpTime);

private:
	uint32_t m_nServerIp;
	int32_t m_nUtcOffset;
	int32_t m_nHandle { -1 };
	int32_t m_nOffsetSeconds { 0 };
	uint32_t m_nOffsetMicros { 0 };
	uint32_t m_MillisRequest { 0 };
	uint32_t m_MillisLastPoll { 0 };

	struct TimeStamp T1 { 0, 0 };	// time request sent by client
	struct TimeStamp T2 { 0, 0 };	// time request received by server
	struct TimeStamp T3 { 0, 0 };	// time reply sent by server
	struct TimeStamp T4 { 0, 0 };	// time reply received by client

	struct ntp::Packet m_Request;

	ntpclient::Status m_Status { ntpclient::Status::STOPPED };

	static NtpClient *s_pThis;
};

#endif /* NTPCLIENT_H_ */
