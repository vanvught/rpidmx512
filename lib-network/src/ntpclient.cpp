/**
 * @file ntpclient.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ntpclient.h"
#include "ntp.h"

#include "network.h"
#include "hardware.h"

#if defined (H3)
 #include "./../lib-h3/include/net/net.h"
 #include "display.h"
 #include "display7segment.h"
#endif

#include "debug.h"

#define RETRIES			3
#define TIMEOUT			3 		// seconds
#define POLL_SECONDS	1024	// 2ˆ10

NtpClient *NtpClient::s_pThis = 0;

NtpClient::NtpClient(uint32_t nServerIp):
	m_nServerIp(nServerIp),
	m_nHandle(-1),
	m_tStatus(NTP_CLIENT_STATUS_STOPPED),
	m_InitTime(0),
	m_RequestTime(0),
	m_LastPoll(0)
{
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		m_nServerIp = Network::Get()->GetNtpServerIp();
	}

	memset(&m_Request, 0, sizeof m_Request);

	m_Request.LiVnMode = NTP_VERSION | NTP_MODE_CLIENT;
	m_Request.Poll = 10; // Poll: 1024 seconds

	memset(&m_Reply, 0, sizeof m_Reply);

	DEBUG_EXIT
}

NtpClient::~NtpClient(void) {
	Network::Get()->End(NTP_UDP_PORT);
}

void NtpClient::Init(void) {
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		DEBUG_EXIT
		return;
	}

	m_nHandle = Network::Get()->Begin(NTP_UDP_PORT);
	assert(m_nHandle != -1);

#if defined (H3)
	Display::Get()->TextStatus("NTP Client", DISPLAY_7SEGMENT_MSG_INFO_NTP);
#endif

	time_t nNow = Hardware::Get()->GetTime();
	uint32_t nRetries;
	uint32_t nBytesReceived;

	for (nRetries = 0; nRetries < RETRIES; nRetries++) {
		Network::Get()->SendTo(m_nHandle, (const uint8_t *)&m_Request, sizeof m_Request, m_nServerIp, NTP_UDP_PORT);

		uint32_t nFromIp;
		uint16_t nFromPort;

		while ((nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *)&m_Reply, sizeof m_Reply, &nFromIp, &nFromPort)) == 0) {
#if defined (H3)
			net_handle();
#endif
			if ((Hardware::Get()->GetTime() - nNow) > TIMEOUT) {
				break;
			}
		}

		if (nBytesReceived == sizeof m_Reply) {
			if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
				DEBUG_PUTS("nFromIp != m_nServerIp");
				continue;
			}

			debug_dump((void *)&m_Reply, sizeof m_Reply);

			if ((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER) {
				m_InitTime = (time_t)(__builtin_bswap32(m_Reply.ReceiveTimestamp_s) - NTP_TIMESTAMP_DELTA);
				DEBUG_PRINTF("m_InitTime=%u", (unsigned) m_InitTime);

				struct tm *pLocalTime = localtime(&m_InitTime);
				struct THardwareTime hwTime;
				memcpy(&hwTime, pLocalTime, sizeof (struct THardwareTime));

				DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

				if(Hardware::Get()->SetTime(hwTime)) {
					m_LastPoll = Hardware::Get()->GetTime();
					m_tStatus = NTP_CLIENT_STATUS_IDLE;
				}

			} else {
				DEBUG_PUTS("!>> Invalid reply <<!");
			}
			break;
		}
	}

	if (m_tStatus == NTP_CLIENT_STATUS_STOPPED) {
#if defined (H3)
		Display::Get()->TextStatus("Error: NTP", DISPLAY_7SEGMENT_MSG_ERROR_NTP);
#endif
		Network::Get()->End(NTP_UDP_PORT);
	}

	DEBUG_PRINTF("nBytesReceived=%d, nRetries=%d, m_tStatus=%d", nBytesReceived, nRetries, (int) m_tStatus);
	DEBUG_EXIT
}

void NtpClient::Run(void) {
	if (m_tStatus == NTP_CLIENT_STATUS_STOPPED) {
		return;
	}

	if (m_tStatus == NTP_CLIENT_STATUS_IDLE) {
		if (__builtin_expect(((Hardware::Get()->GetTime() - m_LastPoll) > POLL_SECONDS), 0)) {
			Network::Get()->SendTo(m_nHandle, (const uint8_t *)&m_Request, sizeof m_Request, m_nServerIp, NTP_UDP_PORT);
			m_RequestTime = Hardware::Get()->GetTime();
			m_tStatus = NTP_CLIENT_STATUS_WAITING;
			DEBUG_PUTS("NTP_CLIENT_STATUS_WAITING");
		}

		return;
	}

	if (m_tStatus == NTP_CLIENT_STATUS_WAITING) {
		uint32_t nFromIp;
		uint16_t nFromPort;

		if (Network::Get()->RecvFrom(m_nHandle, (uint8_t *)&m_Reply, sizeof m_Reply, &nFromIp, &nFromPort) != sizeof m_Reply) {
			if (__builtin_expect(((Hardware::Get()->GetTime() - m_RequestTime) > TIMEOUT), 0)) {
				Network::Get()->End(NTP_UDP_PORT);
				m_tStatus = NTP_CLIENT_STATUS_STOPPED;
#if defined (H3)
				Display::Get()->TextStatus("Error: NTP", DISPLAY_7SEGMENT_MSG_ERROR_NTP);
#endif
				DEBUG_PUTS("NTP_CLIENT_STATUS_STOPPED");
			}
			return;
		}

		if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
			DEBUG_PUTS("nFromIp != m_nServerIp");
			return;
		}

		debug_dump((void *)&m_Reply, sizeof m_Reply);

		if (__builtin_expect(((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER), 1)) {
			const time_t nTime = (time_t)(__builtin_bswap32(m_Reply.ReceiveTimestamp_s) - NTP_TIMESTAMP_DELTA);
			Hardware::Get()->SetSysTime(nTime);
			m_LastPoll = Hardware::Get()->GetTime();

#ifndef NDEBUG
			DEBUG_PRINTF("nTime=%u", (unsigned) nTime);
			struct tm *pLocalTime = localtime(&nTime);
			struct THardwareTime hwTime;
			memcpy(&hwTime, pLocalTime, sizeof (struct THardwareTime));
			DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
		} else {
			DEBUG_PUTS("!>> Invalid reply <<!");
		}

		m_tStatus = NTP_CLIENT_STATUS_IDLE;
		DEBUG_PUTS("NTP_CLIENT_STATUS_IDLE");
	}
}

void NtpClient::Print(void) {
	printf("NTP v%d Client\n", NTP_VERSION >> 3);
	if (m_nServerIp == 0) {
		printf(" Not enabled\n");
		return;
	}
	printf(" Server : " IPSTR "\n", IP2STR(m_nServerIp));
	printf(" Port   : %d\n", NTP_UDP_PORT);
	printf(" Status : %d%c\n", (int) m_tStatus, m_tStatus == NTP_CLIENT_STATUS_STOPPED ? '!' : ' ');
	printf(" Time   : %s", asctime(localtime((const time_t *) &m_InitTime)));
}
