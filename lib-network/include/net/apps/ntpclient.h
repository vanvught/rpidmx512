/**
 * @file ntpclient.h
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

#ifndef NET_APPS_NTPCLIENT_H_
#define NET_APPS_NTPCLIENT_H_

#include <cstdint>
#include <cstdio>
#include <time.h>

#include "net/protocol/ntp.h"
#include "hardware.h"

#include "debug.h"

#if !defined(CONFIG_NTP_CLIENT_POLL_POWER)
# define CONFIG_NTP_CLIENT_POLL_POWER 10
#endif

namespace ntpclient {
static constexpr uint32_t TIMEOUT_MILLIS = 3000;
static constexpr uint8_t POLL_POWER = CONFIG_NTP_CLIENT_POLL_POWER;
static constexpr uint32_t POLL_SECONDS = (1U << POLL_POWER);

void display_status(const ::ntp::Status status);
}  // namespace ntpclient

class NtpClient {
public:
	NtpClient();

	void Start();
	void Stop();
	void Print();

	void SetServerIp(const uint32_t nServerIp) {
		m_nServerIp = nServerIp;
	}

	ntp::Status GetStatus() const {
		return m_Status;
	}

	void Run() {
		if (m_Status == ntp::Status::STOPPED) {
			return;
		}

		if ((m_Status == ntp::Status::IDLE) || (m_Status == ntp::Status::FAILED)) {
			if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisLastPoll) > (1000 * ntpclient::POLL_SECONDS)), 0)) {
				Send();
				m_MillisRequest = Hardware::Get()->Millis();
				m_Status = ntp::Status::WAITING;
				ntpclient::display_status(ntp::Status::WAITING);
				DEBUG_PUTS("ntp::Status::WAITING");
			}

			return;
		}

		if (m_Status == ntp::Status::WAITING) {
			uint8_t LiVnMode;

			if (!Receive(LiVnMode)) {
				if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisRequest) > ntpclient::TIMEOUT_MILLIS), 0)) {
					m_Status = ntp::Status::FAILED;
					ntpclient::display_status(ntp::Status::FAILED);
					DEBUG_PUTS("ntp::Status::FAILED");
				}

				return;
			}

			if (__builtin_expect(((LiVnMode & ntp::MODE_SERVER) == ntp::MODE_SERVER), 1)) {
				m_MillisLastPoll = Hardware::Get()->Millis();

				SetTimeOfDay();
#ifndef NDEBUG
				const auto nTime = time(nullptr);
				const auto *pLocalTime = localtime(&nTime);
				DEBUG_PRINTF("localtime: %.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year + 1900, pLocalTime->tm_mon + 1, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
			} else {
				DEBUG_PUTS("!>> Invalid reply <<!");
			}

			m_Status = ntp::Status::IDLE;
			ntpclient::display_status(ntp::Status::IDLE);
			DEBUG_PUTS("ntp::Status::IDLE");
		}
	}

	static NtpClient *Get() {
		return s_pThis;
	}

private:
	void GetTimeNtpFormat(uint32_t &nSeconds, uint32_t &nFraction);
	void Send();
	bool Receive(uint8_t& LiVnMode);

	void Difference(const struct ntp::TimeStamp& Start, const struct ntp::TimeStamp& Stop, int32_t &nDiffSeconds, int32_t &nDiffMicroSeconds);
	void SetTimeOfDay();

	void PrintNtpTime(const char *pText, const struct ntp::TimeStamp *pNtpTime);

private:
	uint32_t m_nServerIp;
	int32_t m_nHandle { -1 };
	uint32_t m_MillisRequest { 0 };
	uint32_t m_MillisLastPoll { 0 };

	struct ntp::TimeStamp T1 { 0, 0 };	// time request sent by client
	struct ntp::TimeStamp T2 { 0, 0 };	// time request received by server
	struct ntp::TimeStamp T3 { 0, 0 };	// time reply sent by server
	struct ntp::TimeStamp T4 { 0, 0 };	// time reply received by client

	struct ntp::Packet m_Request;

	ntp::Status m_Status { ntp::Status::STOPPED };

	static NtpClient *s_pThis;
};

#endif /* NET_APPS_NTPCLIENT_H_ */
