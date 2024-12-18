/**
 * @file ntpserver.cpp
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

/*
 * https://tools.ietf.org/html/rfc5905
 */

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <cassert>

#include "ntpserver.h"
#include "net/protocol/ntp.h"

#include "network.h"

#include "debug.h"

ntp::Packet NtpServer::s_Reply;
NtpServer *NtpServer::s_pThis;

NtpServer::NtpServer(const uint32_t nYear, const uint32_t nMonth, const uint32_t nDay) {
	DEBUG_ENTRY
	DEBUG_PRINTF("year=%u, month=%u, day=%u", nYear, nMonth, nDay);

	assert(s_pThis == nullptr);
	s_pThis = this;

	struct tm timeDate;

	memset(&timeDate, 0, sizeof(struct tm));
	timeDate.tm_year = static_cast<int>(100 + nYear);
	timeDate.tm_mon = static_cast<int>(nMonth - 1);
	timeDate.tm_mday = static_cast<int>(nDay);

	m_Time = mktime(&timeDate);
	assert(m_Time != -1);

	DEBUG_PRINTF("m_Time=%.8x %ld", static_cast<unsigned int>(m_Time), m_Time);

	m_Time += static_cast<time_t>(ntp::JAN_1970);

	DEBUG_PRINTF("m_Time=%.8x %ld", static_cast<unsigned int>(m_Time), m_Time);
	DEBUG_EXIT
}

NtpServer::~NtpServer() {
	Stop();
}

void NtpServer::Start() {
	DEBUG_ENTRY

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->Begin(ntp::UDP_PORT);
	assert(m_nHandle != -1);

	s_Reply.LiVnMode = ntp::VERSION | ntp::MODE_SERVER;
	s_Reply.Stratum = ntp::STRATUM;
	s_Reply.Poll = ntp::MINPOLL;
	s_Reply.Precision = static_cast<uint8_t>(-10);	// -9.9 = LOG2(0.0001) -> milliseconds
	s_Reply.RootDelay = 0;
	s_Reply.RootDispersion = 0;
	s_Reply.ReferenceID = Network::Get()->GetIp();

	DEBUG_EXIT
}

void NtpServer::Stop() {
	DEBUG_ENTRY

	assert(m_nHandle != -1);
	Network::Get()->End(ntp::UDP_PORT);
	m_nHandle = -1;

	DEBUG_EXIT
}

void NtpServer::SetTimeCode(const struct ltc::TimeCode *pLtcTimeCode) {
	m_tTimeDate = m_Time;
	m_tTimeDate += pLtcTimeCode->nSeconds;
	m_tTimeDate += pLtcTimeCode->nMinutes * 60;
	m_tTimeDate += pLtcTimeCode->nHours * 60 * 60;

	const auto type = static_cast<ltc::Type>(pLtcTimeCode->nType);

	if (type == ltc::Type::FILM) {
		m_nFraction = static_cast<uint32_t>((178956970.625 * pLtcTimeCode->nFrames));
	} else if (type == ltc::Type::EBU) {
		m_nFraction = static_cast<uint32_t>((171798691.8 * pLtcTimeCode->nFrames));
	} else if ((type == ltc::Type::DF) || (type == ltc::Type::SMPTE)) {
		m_nFraction = static_cast<uint32_t>((143165576.5 * pLtcTimeCode->nFrames));
	} else {
		assert(0);
	}

	s_Reply.ReferenceTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	s_Reply.ReferenceTimestamp_f = __builtin_bswap32(m_nFraction);
	s_Reply.ReceiveTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	s_Reply.ReceiveTimestamp_f = __builtin_bswap32(m_nFraction);
	s_Reply.TransmitTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(m_tTimeDate));
	s_Reply.TransmitTimestamp_f = __builtin_bswap32(m_nFraction);
}

void NtpServer::Print() {
	printf("NTP v%d Server\n", ntp::VERSION >> 3);
	printf(" Port : %d\n", ntp::UDP_PORT);
	printf(" Stratum : %d\n", ntp::STRATUM);

	const auto t = static_cast<time_t>(static_cast<uint32_t>(m_Time) - ntp::JAN_1970);

	printf(" %s", asctime(localtime(&t)));
}
