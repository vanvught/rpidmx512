/**
 * @file ntpserver.cpp
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

/*
 * https://tools.ietf.org/html/rfc5905
 */

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cassert>

#include "ntpserver.h"

#include "network.h"
#include "ntp.h"

#include "debug.h"

NtpServer *NtpServer::s_pThis = nullptr;

NtpServer::NtpServer(uint8_t nYear, uint8_t nMonth, uint8_t nDay) {
	DEBUG_ENTRY
	DEBUG_PRINTF("year=%d, month=%d, day=%d", nYear, nMonth, nDay);

	assert(s_pThis == nullptr);
	s_pThis = this;

	struct tm timeDate;

	memset(&timeDate, 0, sizeof(struct tm));
	timeDate.tm_year = 100 + nYear;
	timeDate.tm_mon = nMonth - 1;
	timeDate.tm_mday = nDay;

	m_tDate = mktime(&timeDate);
	assert(m_tDate != -1);

	DEBUG_PRINTF("m_tDate=%.8x %ld", static_cast<unsigned int>(m_tDate), m_tDate);

	m_tDate += NTP_TIMESTAMP_DELTA;

	DEBUG_PRINTF("m_tDate=%.8x %ld", static_cast<unsigned int>(m_tDate), m_tDate);
	DEBUG_EXIT
}

NtpServer::~NtpServer() {
	Stop();
}

void NtpServer::Start() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(NTP_UDP_PORT);
	assert(m_nHandle != -1);

	m_Reply.LiVnMode = NTP_VERSION | NTP_MODE_SERVER;

	m_Reply.Stratum = NTP_STRATUM;
	m_Reply.Poll = NTP_MINPOLL;
	m_Reply.Precision = static_cast<uint8_t>(-10);	// -9.9 = LOG2(0.0001) -> milliseconds
	m_Reply.RootDelay = 0;
	m_Reply.RootDispersion = 0;
	m_Reply.ReferenceID = Network::Get()->GetIp();

	DEBUG_EXIT
}

void NtpServer::Stop() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->End(NTP_UDP_PORT);

	DEBUG_EXIT
}

void NtpServer::SetTimeCode(const struct TLtcTimeCode *pLtcTimeCode) {
	m_tTimeDate = m_tDate;
	m_tTimeDate += pLtcTimeCode->nSeconds;
	m_tTimeDate += pLtcTimeCode->nMinutes * 60;
	m_tTimeDate += pLtcTimeCode->nHours * 60 * 60;

	if (pLtcTimeCode->nType == ltc::type::FILM) {
		m_nFraction = static_cast<uint32_t>((178956970.625 * pLtcTimeCode->nFrames));
	} else if (pLtcTimeCode->nType == ltc::type::EBU) {
		m_nFraction = static_cast<uint32_t>((171798691.8 * pLtcTimeCode->nFrames));
	} else if ((pLtcTimeCode->nType == ltc::type::DF) || (pLtcTimeCode->nType == ltc::type::SMPTE)) {
		m_nFraction = static_cast<uint32_t>((143165576.5 * pLtcTimeCode->nFrames));
	} else {
		assert(0);
	}

//	DEBUG_PRINTF("m_timeDate=%.8x %ld", static_cast<unsigned int>(m_tTimeDate), m_tTimeDate);

	m_Reply.ReferenceTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	m_Reply.ReferenceTimestamp_f = __builtin_bswap32(m_nFraction);
	m_Reply.ReceiveTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	m_Reply.ReceiveTimestamp_f = __builtin_bswap32(m_nFraction);
	m_Reply.TransmitTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	m_Reply.TransmitTimestamp_f = __builtin_bswap32(m_nFraction);
}

void NtpServer::Run() {
	uint32_t nIPAddressFrom;
	uint16_t nForeignPort;

	const uint16_t nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &m_Request, sizeof(struct TNtpPacket), &nIPAddressFrom, &nForeignPort);

	if (__builtin_expect((nBytesReceived < sizeof(struct TNtpPacket)), 1)) {
		return;
	}

	if (__builtin_expect(((m_Request.LiVnMode & NTP_MODE_CLIENT) != NTP_MODE_CLIENT), 0)) {
		return;
	}

	m_Reply.OriginTimestamp_s = m_Request.TransmitTimestamp_s;
	m_Reply.OriginTimestamp_f = m_Request.TransmitTimestamp_f;

	Network::Get()->SendTo(m_nHandle, &m_Reply, sizeof(struct TNtpPacket), nIPAddressFrom, nForeignPort);
}

void NtpServer::Print() {
	printf("NTP v%d Server\n", NTP_VERSION >> 3);
	printf(" Port : %d\n", NTP_UDP_PORT);
	printf(" Stratum : %d\n", NTP_STRATUM);

	const time_t t = m_tDate - NTP_TIMESTAMP_DELTA;

	printf(" %s", asctime(localtime(&t)));
}
