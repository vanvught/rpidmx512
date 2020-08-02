/**
 * @file ntpclient.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include "ntpclient.h"
#include "ntp.h"

#include "utc.h"

#include "network.h"
#include "hardware.h"

#if defined (H3)
extern "C" {
 void net_handle(void);
}
#endif

#include "debug.h"

#define RETRIES			3
#define TIMEOUT_MILLIS	3000 	// 3 seconds
#define POLL_SECONDS	1024	// 2ˆ10

NtpClient::NtpClient(uint32_t nServerIp):
	m_nServerIp(nServerIp),
	m_nHandle(-1),
	m_tStatus(NtpClientStatus::STOPPED),
	m_InitTime(0),
	m_MillisRequest(0),
	m_MillisLastPoll(0)
{
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		m_nServerIp = Network::Get()->GetNtpServerIp();
	}

	SetUtcOffset(Network::Get()->GetNtpUtcOffset());

	memset(&m_Request, 0, sizeof m_Request);

	m_Request.LiVnMode = NTP_VERSION | NTP_MODE_CLIENT;
	m_Request.Poll = 10; // Poll: 1024 seconds

	memset(&m_Reply, 0, sizeof m_Reply);

	DEBUG_EXIT
}

void NtpClient::SetUtcOffset(float fUtcOffset) {
	// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
	m_nUtcOffset = Utc::Validate(fUtcOffset);
}

void NtpClient::Init() {
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		DEBUG_EXIT
		return;
	}

	m_nHandle = Network::Get()->Begin(NTP_UDP_PORT);
	assert(m_nHandle != -1);

	if (m_pNtpClientDisplay != nullptr) {
		m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::INIT);
	}

	const uint32_t nNow = Hardware::Get()->Millis();
	uint32_t nRetries;
	uint16_t nBytesReceived;

	for (nRetries = 0; nRetries < RETRIES; nRetries++) {
		Network::Get()->SendTo(m_nHandle, &m_Request, sizeof m_Request, m_nServerIp, NTP_UDP_PORT);

		uint32_t nFromIp;
		uint16_t nFromPort;

		while ((nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Reply, sizeof m_Reply, &nFromIp, &nFromPort)) == 0) {
#if defined (H3)
			net_handle();
#endif
			if ((Hardware::Get()->Millis() - nNow) > TIMEOUT_MILLIS) {
				break;
			}
		}

		if (nBytesReceived == sizeof m_Reply) {
			if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
				DEBUG_PUTS("nFromIp != m_nServerIp");
				continue;
			}

			debug_dump(&m_Reply, sizeof m_Reply);

			if ((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER) {
				m_InitTime = ((__builtin_bswap32(m_Reply.ReceiveTimestamp_s) - NTP_TIMESTAMP_DELTA + m_nUtcOffset));
				DEBUG_PRINTF("m_InitTime=%u", static_cast<unsigned>(m_InitTime));

				struct tm *pLocalTime = localtime(&m_InitTime);

				DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

				if (Hardware::Get()->SetTime(pLocalTime)) {
					m_MillisLastPoll = Hardware::Get()->Millis();
					m_tStatus = NtpClientStatus::IDLE;
				}
			} else {
				DEBUG_PUTS("!>> Invalid reply <<!");
			}
			break;
		}
	}

	if (m_tStatus == NtpClientStatus::STOPPED) {
		if (m_pNtpClientDisplay != nullptr) {
			m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::STOPPED);
		}
	}

	DEBUG_PRINTF("nBytesReceived=%d, nRetries=%d, m_tStatus=%d", nBytesReceived, nRetries, static_cast<int>(m_tStatus));
	DEBUG_EXIT
}

void NtpClient::Run() {
	if (m_tStatus == NtpClientStatus::STOPPED) {
		return;
	}

	if (m_tStatus == NtpClientStatus::IDLE) {
		if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisLastPoll) > (1000 * POLL_SECONDS)), 0)) {
			Network::Get()->SendTo(m_nHandle, &m_Request, sizeof m_Request, m_nServerIp, NTP_UDP_PORT);
			m_MillisRequest = Hardware::Get()->Millis();
			m_tStatus = NtpClientStatus::WAITING;
			DEBUG_PUTS("NtpClientStatus::WAITING");
		}

		return;
	}

	if (m_tStatus == NtpClientStatus::WAITING) {
		uint32_t nFromIp;
		uint16_t nFromPort;

		if ((Network::Get()->RecvFrom(m_nHandle, &m_Reply, sizeof m_Reply, &nFromIp, &nFromPort)) != sizeof m_Reply) {
			if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisRequest) > TIMEOUT_MILLIS), 0)) {
				m_tStatus = NtpClientStatus::STOPPED;
				if (m_tStatus == NtpClientStatus::STOPPED) {
					if (m_pNtpClientDisplay != nullptr) {
						m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::STOPPED);
					}
				}
				DEBUG_PUTS("NtpClientStatus::STOPPED");
			}
			return;
		}

		if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
			DEBUG_PUTS("nFromIp != m_nServerIp");
			return;
		}

		debug_dump(&m_Reply, sizeof m_Reply);

		if (__builtin_expect(((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER), 1)) {
			const time_t nTime = ((__builtin_bswap32(m_Reply.ReceiveTimestamp_s) - NTP_TIMESTAMP_DELTA + m_nUtcOffset));
			Hardware::Get()->SetSysTime(nTime);
			m_MillisLastPoll = Hardware::Get()->Millis();

#ifndef NDEBUG
			DEBUG_PRINTF("nTime=%u", static_cast<unsigned>(nTime));
			struct tm *pLocalTime = localtime(&nTime);
			DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
		} else {
			DEBUG_PUTS("!>> Invalid reply <<!");
		}

		m_tStatus = NtpClientStatus::IDLE;
		DEBUG_PUTS("NtpClientStatus::IDLE");
	}
}

void NtpClient::Print() {
	printf("NTP v%d Client\n", NTP_VERSION >> 3);
	if (m_nServerIp == 0) {
		printf(" Not enabled\n");
		return;
	}
	printf(" Server : " IPSTR "\n", IP2STR(m_nServerIp));
	printf(" Port : %d\n", NTP_UDP_PORT);
	printf(" Status : %d%c\n", static_cast<int>(m_tStatus), m_tStatus == NtpClientStatus::STOPPED ? '!' : ' ');
	printf(" Time : %s", asctime(localtime(&m_InitTime)));
	printf(" UTC offset : %d (seconds)\n", m_nUtcOffset);
}
