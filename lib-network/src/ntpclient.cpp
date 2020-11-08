/**
 * @file ntpclient.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
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

static constexpr auto RETRIES = 3;
static constexpr auto TIMEOUT_MILLIS = 3000; 	// 3 seconds
static constexpr auto POLL_SECONDS = 1024;		// 2ˆ10
static constexpr auto JAN_1970 = 0x83aa7e80; 	// 2208988800 1970 - 1900 in seconds

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294U*static_cast<uint32_t>(x) + ( (1981U*static_cast<uint32_t>(x))>>11 ) +  ((2911U*static_cast<uint32_t>(x))>>28) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/*
Timestamp Name        ID   When Generated
----------------------------------------------------------------
Originate Timestamp   T1   time request sent by client
Receive Timestamp     T2   time request received by server
Transmit Timestamp    T3   time reply sent by server
Destination Timestamp T4   time reply received by client
*/

NtpClient *NtpClient::s_pThis = nullptr;

NtpClient::NtpClient(uint32_t nServerIp): m_nServerIp(nServerIp) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	if (m_nServerIp == 0) {
		m_nServerIp = Network::Get()->GetNtpServerIp();
	}

	SetUtcOffset(Network::Get()->GetNtpUtcOffset());

	memset(&m_Request, 0, sizeof m_Request);
	memset(&m_Reply, 0, sizeof m_Reply);

	m_Request.LiVnMode = NTP_VERSION | NTP_MODE_CLIENT;
	m_Request.Poll = 10; // Poll: 1024 seconds
	m_Request.ReferenceID = ('A' << 0) | ('V' << 8) | ('S' << 16);

	DEBUG_EXIT
}

void NtpClient::SetUtcOffset(float fUtcOffset) {
	// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
	m_nUtcOffset = Utc::Validate(fUtcOffset);
}

/*
 * Seconds and Fractions since 01.01.1900
 */
void NtpClient::GetTimeNtpFormat(uint32_t &nSeconds, uint32_t &nFraction) {
	struct timeval now;
	gettimeofday(&now, nullptr);
	nSeconds = static_cast<uint32_t>(now.tv_sec - m_nUtcOffset) + JAN_1970;
	nFraction = NTPFRAC(now.tv_usec);
}

void NtpClient::Send() {
	GetTimeNtpFormat(T1.nSeconds, T1.nFraction);

	m_Request.OriginTimestamp_s = __builtin_bswap32(T1.nSeconds);
	m_Request.OriginTimestamp_f = __builtin_bswap32(T1.nFraction);

	Network::Get()->SendTo(m_nHandle, &m_Request, sizeof m_Request, m_nServerIp, NTP_UDP_PORT);
}

bool NtpClient::Receive() {
	uint32_t nFromIp;
	uint16_t nFromPort;

	const uint16_t nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Reply, sizeof m_Reply, &nFromIp, &nFromPort);

	if (__builtin_expect((nBytesReceived != sizeof m_Reply), 1)) {
		return false;
	}

	GetTimeNtpFormat(T4.nSeconds, T4.nFraction);

	if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
		DEBUG_PUTS("nFromIp != m_nServerIp");
		return false;
	}

	T2.nSeconds = __builtin_bswap32(m_Reply.ReceiveTimestamp_s);
	T2.nFraction = __builtin_bswap32(m_Reply.ReceiveTimestamp_f);

	T3.nSeconds = __builtin_bswap32(m_Reply.TransmitTimestamp_s);
	T3.nFraction = __builtin_bswap32(m_Reply.TransmitTimestamp_f);

	PrintNtpTime("Originate", &T1);
	PrintNtpTime("Receive", &T2);
	PrintNtpTime("Transmit", &T3);
	PrintNtpTime("Destination", &T4);

	return true;
}

void NtpClient::Difference(const struct TimeStamp *Start, const struct TimeStamp *Stop, int32_t &nDiffSeconds, uint32_t &nDiffMicros) {
	nDiffSeconds = static_cast<int32_t>(Stop->nSeconds - Start->nSeconds);
	uint32_t nDiffFraction;

	if (Stop->nFraction >= Start->nFraction) {
		nDiffFraction = Stop->nFraction - Start->nFraction;
	} else {
		nDiffFraction = Start->nFraction - Stop->nFraction;
		nDiffFraction = ~nDiffFraction;
		nDiffSeconds -= 1;
	}

	nDiffMicros = USEC(nDiffFraction);

	DEBUG_PRINTF("Seconds  %u - %u = %d", Stop->nSeconds, Start->nSeconds, nDiffSeconds);
	DEBUG_PRINTF("Micros   %u - %u = %u", USEC(Stop->nFraction), USEC(Start->nFraction), nDiffMicros);
}

int NtpClient::SetTimeOfDay() {
	int32_t nDiffSeconds1, nDiffSeconds2 ;
	uint32_t nDiffFraction1, nDiffFraction2;

	Difference(&T1, &T2, nDiffSeconds1, nDiffFraction1);
	Difference(&T4, &T3, nDiffSeconds2, nDiffFraction2);

	m_nOffsetSeconds = nDiffSeconds1 + nDiffSeconds2;
	m_nOffsetMicros = nDiffFraction1 + nDiffFraction2;

	if (m_nOffsetMicros >= 1E6) {
		m_nOffsetMicros -= 1E6	;
		m_nOffsetSeconds += 1;
	}

	m_nOffsetSeconds /= 2;
	m_nOffsetMicros /= 2;

	struct timeval tv;

	tv.tv_sec =	static_cast<time_t>(T4.nSeconds - JAN_1970)  + m_nOffsetSeconds + m_nUtcOffset;
	tv.tv_usec = static_cast<suseconds_t>(static_cast<int32_t>(USEC(T4.nFraction)) + static_cast<int32_t>(m_nOffsetMicros));

	if (tv.tv_usec >= 1E6) {
		tv.tv_sec++;
		tv.tv_usec = 1E6 - tv.tv_usec;
	}

	DEBUG_PRINTF("(%ld, %d) %s ", tv.tv_sec, static_cast<int>(tv.tv_usec) , tv.tv_usec  >= 1E6 ? "!" : "");
	DEBUG_PRINTF("%d %u",m_nOffsetSeconds, m_nOffsetMicros);

	return settimeofday(&tv, nullptr);
}

void NtpClient::Start() {
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		m_tStatus = NtpClientStatus::FAILED;
		DEBUG_EXIT
		return;
	}

	if ((m_tStatus != NtpClientStatus::IDLE) && (m_tStatus != NtpClientStatus::STOPPED)) {
		DEBUG_EXIT
		return;
	}

	m_nHandle = Network::Get()->Begin(NTP_UDP_PORT);
	assert(m_nHandle != -1);

	if (m_pNtpClientDisplay != nullptr) {
		m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::IDLE);
	}

	const uint32_t nNow = Hardware::Get()->Millis();
	uint32_t nRetries;

	for (nRetries = 0; nRetries < RETRIES; nRetries++) {
		Send();

		while (!Receive()) {
#if defined (H3)
			net_handle();
#endif
			if ((Hardware::Get()->Millis() - nNow) > TIMEOUT_MILLIS) {
				break;
			}
		}

		if ((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER) {
			if (SetTimeOfDay() == 0) {
				m_tStatus = NtpClientStatus::IDLE;
			} else {
				// Error
			}
		} else {
			DEBUG_PUTS("!>> Invalid reply <<!");
		}

		break;
	}

	m_MillisLastPoll = Hardware::Get()->Millis();

	if (nRetries == RETRIES) {
		m_tStatus = NtpClientStatus::FAILED;

		if (m_pNtpClientDisplay != nullptr) {
			m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::FAILED);
		}
	}


	DEBUG_PRINTF("nRetries=%d, m_tStatus=%d", nRetries, static_cast<int>(m_tStatus));
	DEBUG_EXIT
}

void NtpClient::Stop() {
	DEBUG_ENTRY

	if (m_tStatus == NtpClientStatus::STOPPED) {
		return;
	}

	m_nHandle = Network::Get()->End(NTP_UDP_PORT);
	m_tStatus = NtpClientStatus::STOPPED;

	if (m_pNtpClientDisplay != nullptr) {
		m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::STOPPED);
	}

	DEBUG_EXIT
}

void NtpClient::Run() {
	if (__builtin_expect((m_nServerIp == 0), 0)) {
		return;
	}

	if ((m_tStatus == NtpClientStatus::IDLE) || (m_tStatus == NtpClientStatus::FAILED)) {
		if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisLastPoll) > (1000 * POLL_SECONDS)), 0)) {
			Send();
			m_MillisRequest = Hardware::Get()->Millis();
			m_tStatus = NtpClientStatus::WAITING;
			DEBUG_PUTS("NtpClientStatus::WAITING");
		}

		return;
	}

	if (m_tStatus == NtpClientStatus::WAITING) {
		if (!Receive()) {
			if (__builtin_expect(((Hardware::Get()->Millis() - m_MillisRequest) > TIMEOUT_MILLIS), 0)) {
				m_tStatus = NtpClientStatus::FAILED;

				if (m_pNtpClientDisplay != nullptr) {
					m_pNtpClientDisplay->ShowNtpClientStatus(NtpClientStatus::FAILED);
				}
				DEBUG_PUTS("NtpClientStatus::FAILED");
			}

			return;
		}

		if (__builtin_expect(((m_Reply.LiVnMode & NTP_MODE_SERVER) == NTP_MODE_SERVER), 1)) {
			m_MillisLastPoll = Hardware::Get()->Millis();

			if (SetTimeOfDay() == 0) {
#ifndef NDEBUG
				const time_t nTime = time(nullptr);
				const struct tm *pLocalTime = localtime(&nTime);
				DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year, pLocalTime->tm_mon, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
			} else {
				// Error
			}
		} else {
			DEBUG_PUTS("!>> Invalid reply <<!");
		}

		m_tStatus = NtpClientStatus::IDLE;
		DEBUG_PUTS("NtpClientStatus::IDLE");
	}
}

void NtpClient::PrintNtpTime(__attribute__((unused)) const char *pText, __attribute__((unused)) const struct TimeStamp *pNtpTime) {
#ifndef NDEBUG
	const auto nSeconds = static_cast<time_t>(pNtpTime->nSeconds - JAN_1970);
	const struct tm *pTm = localtime(&nSeconds);
	printf("%s %02d:%02d:%02d.%06d %04d [%u]\n", pText, pTm->tm_hour, pTm->tm_min,  pTm->tm_sec, USEC(pNtpTime->nFraction), pTm->tm_year + 1900, pNtpTime->nSeconds);
#endif
}

void NtpClient::Print() {
	printf("NTP v%d Client\n", NTP_VERSION >> 3);
	if (m_nServerIp == 0) {
		printf(" Not enabled\n");
		return;
	}
	printf(" Server : " IPSTR ":%d\n", IP2STR(m_nServerIp), NTP_UDP_PORT);
	printf(" Status : %d\n", static_cast<int>(m_tStatus));
	printf(" UTC offset : %d (seconds)\n", m_nUtcOffset);
	// Debug ONLY
	PrintNtpTime("Originate", &T1);
	PrintNtpTime("Receive", &T2);
	PrintNtpTime("Transmit", &T3);
	PrintNtpTime("Destination", &T4);
}
