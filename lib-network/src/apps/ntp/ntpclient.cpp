/**
 * @file ntpclient.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <cassert>

#include "ntpclient.h"
#include "ntp.h"

#include "utc.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

static constexpr auto RETRIES = 3;

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

NtpClient *NtpClient::s_pThis;

NtpClient::NtpClient(const uint32_t nServerIp):
	m_nServerIp(nServerIp == 0 ? Network::Get()->GetNtpServerIp() : nServerIp)
{
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
	m_nUtcOffset = hal::utc_validate((Network::Get()->GetNtpUtcOffset()));

	memset(&m_Request, 0, sizeof m_Request);

	m_Request.LiVnMode = ntp::VERSION | ntp::MODE_CLIENT;
	m_Request.Poll = ntpclient::POLL_POWER;
	m_Request.ReferenceID = ('A' << 0) | ('V' << 8) | ('S' << 16);

	DEBUG_EXIT
}

/*
 * Seconds and Fractions since 01.01.1900
 */
void NtpClient::GetTimeNtpFormat(uint32_t &nSeconds, uint32_t &nFraction) {
	struct timeval now;
	gettimeofday(&now, nullptr);
	nSeconds = static_cast<uint32_t>(now.tv_sec - m_nUtcOffset) + ntp::JAN_1970;
	nFraction = NTPFRAC(now.tv_usec);
}

void NtpClient::Send() {
	GetTimeNtpFormat(T1.nSeconds, T1.nFraction);

	m_Request.OriginTimestamp_s = __builtin_bswap32(T1.nSeconds);
	m_Request.OriginTimestamp_f = __builtin_bswap32(T1.nFraction);

	Network::Get()->SendTo(m_nHandle, &m_Request, sizeof m_Request, m_nServerIp, ntp::UDP_PORT);
}

bool NtpClient::Receive(uint8_t& LiVnMode) {
	uint32_t nFromIp;
	uint16_t nFromPort;
	ntp::Packet *pReply;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&pReply)), &nFromIp, &nFromPort);

	if (__builtin_expect((nBytesReceived != sizeof(struct ntp::Packet)), 1)) {
		return false;
	}

	GetTimeNtpFormat(T4.nSeconds, T4.nFraction);

	if (__builtin_expect((nFromIp != m_nServerIp), 0)) {
		DEBUG_PUTS("nFromIp != m_nServerIp");
		return false;
	}

	LiVnMode = pReply->LiVnMode;

	T2.nSeconds = __builtin_bswap32(pReply->ReceiveTimestamp_s);
	T2.nFraction = __builtin_bswap32(pReply->ReceiveTimestamp_f);

	T3.nSeconds = __builtin_bswap32(pReply->TransmitTimestamp_s);
	T3.nFraction = __builtin_bswap32(pReply->TransmitTimestamp_f);

	PrintNtpTime("T1: ", &T1);
	PrintNtpTime("T2: ", &T2);
	PrintNtpTime("T3: ", &T3);
	PrintNtpTime("T4: ", &T4);

	return true;
}

void NtpClient::Difference(const struct ntp::TimeStamp& Start, const struct ntp::TimeStamp& Stop, int32_t &nDiffSeconds, int32_t &nDiffMicroSeconds) {
    ntp::time_t r;
	const ntp::time_t x = {.tv_sec = static_cast<int32_t>(Stop.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Stop.nFraction))};
	const ntp::time_t y = {.tv_sec = static_cast<int32_t>(Start.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Start.nFraction))};
    ntp::sub_time(&r, &x, &y);

    nDiffSeconds = r.tv_sec;
    nDiffMicroSeconds = r.tv_usec;
}

void NtpClient::SetTimeOfDay() {
	int32_t nDiffSeconds1, nDiffSeconds2 ;
	int32_t nDiffMicroSeconds1, nDiffMicroSeconds2;

	Difference(T1, T2, nDiffSeconds1, nDiffMicroSeconds1);
	Difference(T4, T3, nDiffSeconds2, nDiffMicroSeconds2);

	auto nOffsetSeconds = static_cast<int64_t>(nDiffSeconds1) + static_cast<int64_t>(nDiffSeconds2);
	auto nOffsetMicroSeconds =  static_cast<int64_t>(nDiffMicroSeconds1) + static_cast<int64_t>(nDiffMicroSeconds2);

	const int32_t nOffsetSecondsAverage = nOffsetSeconds / 2;
	const int32_t nOffsetMicrosAverage  = nOffsetMicroSeconds / 2;

	ntp::time_t ptpOffset = {.tv_sec = nOffsetSecondsAverage, .tv_usec = nOffsetMicrosAverage};
	ntp::normalize_time(&ptpOffset);

	struct timeval tv;

	tv.tv_sec =	static_cast<time_t>(T4.nSeconds - ntp::JAN_1970) + nOffsetSecondsAverage + m_nUtcOffset;
	tv.tv_usec = (static_cast<int32_t>(USEC(T4.nFraction)) + static_cast<int32_t>(nOffsetMicrosAverage));

	settimeofday(&tv, nullptr);

#ifndef NDEBUG
	/* delay */
	ntp::time_t diff1;
	ntp::time_t diff2;

	Difference(T1, T4, diff1.tv_sec, diff1.tv_usec);
	Difference(T2, T3, diff2.tv_sec, diff2.tv_usec);

	ntp::time_t ntpDelay;
	ntp::sub_time(&ntpDelay, &diff1, &diff2);

	char sign = '+';

	if (ptpOffset.tv_sec < 0) {
		ptpOffset.tv_sec = -ptpOffset.tv_sec;
		sign = '-';
	}

	if (ptpOffset.tv_usec < 0) {
		ptpOffset.tv_usec = -ptpOffset.tv_usec;
		sign = '-';
	}

	printf(" offset=%c%d.%06d delay=%d.%06d\n", sign, ptpOffset.tv_sec, ptpOffset.tv_usec, ntpDelay.tv_sec, ntpDelay.tv_usec);
#endif
}

void NtpClient::Start() {
	DEBUG_ENTRY

	if (m_nServerIp == 0) {
		m_Status = ntp::Status::STOPPED;
		ntpclient::display_status(ntp::Status::STOPPED);
		DEBUG_EXIT
		return;
	}

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->Begin(ntp::UDP_PORT);
	assert(m_nHandle != -1);

	ntpclient::display_status(m_Status);

	const auto nNow = Hardware::Get()->Millis();
	uint32_t nRetries;

	for (nRetries = 0; nRetries < RETRIES; nRetries++) {
		Send();

		uint8_t LiVnMode = 0;

		while (!Receive(LiVnMode)) {
#if defined (HAVE_NET_HANDLE)
			net_handle();
#endif
			if ((Hardware::Get()->Millis() - nNow) > ntpclient::TIMEOUT_MILLIS) {
				break;
			}
		}

		if ((LiVnMode & ntp::MODE_SERVER) == ntp::MODE_SERVER) {
			SetTimeOfDay();
			m_Status = ntp::Status::IDLE;
		} else {
			m_Status = ntp::Status::FAILED;
			DEBUG_PUTS("!>> Invalid reply <<!");
		}

		break;
	}

	m_MillisLastPoll = Hardware::Get()->Millis();

	if (nRetries == RETRIES) {
		m_Status = ntp::Status::FAILED;
	}

	DEBUG_PRINTF("nRetries=%d, m_Status=%d", nRetries, static_cast<int>(m_Status));

#if !defined(DISABLE_RTC)
	if (m_Status != ntp::Status::FAILED) {
		printf("Set RTC from System Clock\n");
		HwClock::Get()->SysToHc();
#ifndef NDEBUG
		const auto rawtime = time(nullptr);
		printf(asctime(localtime(&rawtime)));
#endif
	}
#endif

	ntpclient::display_status(m_Status);

	DEBUG_EXIT
}

void NtpClient::Stop() {
	DEBUG_ENTRY

	if (m_Status == ntp::Status::STOPPED) {
		return;
	}

	assert(m_nHandle != -1);
	Network::Get()->End(ntp::UDP_PORT);
	m_nHandle = -1;

	m_Status = ntp::Status::STOPPED;

	ntpclient::display_status(ntp::Status::STOPPED);

	DEBUG_EXIT
}

void NtpClient::PrintNtpTime([[maybe_unused]] const char *pText, [[maybe_unused]] const struct ntp::TimeStamp *pNtpTime) {
#ifndef NDEBUG
	const auto nSeconds = static_cast<time_t>(pNtpTime->nSeconds - ntp::JAN_1970);
	const auto *pTm = localtime(&nSeconds);
	printf("%s %02d:%02d:%02d.%06d %04d [%u]\n", pText, pTm->tm_hour, pTm->tm_min,  pTm->tm_sec, USEC(pNtpTime->nFraction), pTm->tm_year + 1900, pNtpTime->nSeconds);
#endif
}

void NtpClient::Print() {
	printf("NTP v%d Client [%s]\n", (ntp::VERSION >> 3), ntp::STATUS[static_cast<int>(m_Status)]);
	if (m_Status == ntp::Status::STOPPED) {
		puts(" Not enabled");
		return;
	}
	printf(" Server : " IPSTR ":%d\n", IP2STR(m_nServerIp), ntp::UDP_PORT);
}
