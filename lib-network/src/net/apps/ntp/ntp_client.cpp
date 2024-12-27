/**
 * @file ntp_client.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_NTP_CLIENT)
# undef NDEBUG
#endif

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include "hardware.h"
#include "network.h"
#include "networkparams.h"

#include "net/protocol/ntp.h"
#include "net/apps/ntp_client.h"

#include "softwaretimers.h"

#include "debug.h"

/*
 * How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294U*static_cast<uint32_t>(x) + ( (1981U*static_cast<uint32_t>(x))>>11 ) +  ((2911U*static_cast<uint32_t>(x))>>28) )

/*
 * The reverse of the above, needed if we want to set our microsecond
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

struct ntpClient {
	uint32_t nServerIp;
	int32_t nHandle;
	TimerHandle_t nTimerId;
	uint32_t nRequestTimeout;
	uint32_t nPollSeconds;
	uint32_t nLockedCount;
	ntp::Status status;
	ntp::Packet Request;
	uint8_t LiVnMode;
	ntp::TimeStamp T1;	// time request sent by client
	ntp::TimeStamp T2;	// time request received by server
	ntp::TimeStamp T3;	// time reply sent by server
	ntp::TimeStamp T4;	// time reply received by client
};

static ntpClient s_ntpClient;
static constexpr uint32_t REQUEST_SIZE = sizeof s_ntpClient.Request;

static void send();

static void ntp_client_timer([[maybe_unused]] TimerHandle_t nHandle) {
	assert(s_ntpClient.status != ntp::Status::STOPPED);
	assert(s_ntpClient.status != ntp::Status::DISABLED);

	if (s_ntpClient.status == ntp::Status::WAITING) {
		if (s_ntpClient.nRequestTimeout > 1) {
			s_ntpClient.nRequestTimeout--;
			return;
		}

		if (s_ntpClient.nRequestTimeout == 1) {
			s_ntpClient.status = ntp::Status::FAILED;
			ntpclient::display_status(ntp::Status::FAILED);
			s_ntpClient.nPollSeconds = ntpclient::POLL_SECONDS_MIN;
			return;
		}
		return;
	}

	if (s_ntpClient.nPollSeconds > 1) {
		s_ntpClient.nPollSeconds--;
		return;
	}

	if (s_ntpClient.nPollSeconds == 1) {
		send();
	}
}

static void print_ntp_time([[maybe_unused]] const char *pText, [[maybe_unused]] const struct ntp::TimeStamp *pNtpTime) {
#ifndef NDEBUG
	const auto nSeconds = static_cast<time_t>(pNtpTime->nSeconds - ntp::JAN_1970);
	const auto *pTm = localtime(&nSeconds);
	printf("%s %02d:%02d:%02d.%06d %04d [%u]\n", pText, pTm->tm_hour, pTm->tm_min,  pTm->tm_sec, USEC(pNtpTime->nFraction), pTm->tm_year + 1900, pNtpTime->nSeconds);
#endif
}

/*
 * Seconds and Fractions since 01.01.1900
 */
static void get_time_ntp_format(uint32_t &nSeconds, uint32_t &nFraction) {
	struct timeval now;
	gettimeofday(&now, nullptr);
	nSeconds = static_cast<uint32_t>(now.tv_sec) + ntp::JAN_1970;
	nFraction = NTPFRAC(now.tv_usec);
}

static void send() {
	get_time_ntp_format(s_ntpClient.T1.nSeconds, s_ntpClient.T1.nFraction);

	s_ntpClient.Request.TransmitTimestamp_s = __builtin_bswap32(s_ntpClient.T1.nSeconds);
	s_ntpClient.Request.TransmitTimestamp_f = __builtin_bswap32(s_ntpClient.T1.nFraction);

	Network::Get()->SendTo(s_ntpClient.nHandle, &s_ntpClient.Request, REQUEST_SIZE, s_ntpClient.nServerIp, ntp::UDP_PORT);

	s_ntpClient.nRequestTimeout = ntpclient::TIMEOUT_SECONDS;
	s_ntpClient.status = ntp::Status::WAITING;
	ntpclient::display_status(ntp::Status::WAITING);
}

static void difference(const struct ntp::TimeStamp& Start, const struct ntp::TimeStamp& Stop, int32_t &nDiffSeconds, int32_t &nDiffMicroSeconds) {
	ntp::time_t r;
	const ntp::time_t x = {.tv_sec = static_cast<int32_t>(Stop.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Stop.nFraction))};
	const ntp::time_t y = {.tv_sec = static_cast<int32_t>(Start.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Start.nFraction))};
	ntp::sub_time(&r, &x, &y);

	nDiffSeconds = r.tv_sec;
	nDiffMicroSeconds = r.tv_usec;
}

static void set_time_of_day() {
	int32_t nDiffSeconds1, nDiffSeconds2 ;
	int32_t nDiffMicroSeconds1, nDiffMicroSeconds2;

	difference(s_ntpClient.T1, s_ntpClient.T2, nDiffSeconds1, nDiffMicroSeconds1);
	difference(s_ntpClient.T4, s_ntpClient.T3, nDiffSeconds2, nDiffMicroSeconds2);

	auto nOffsetSeconds = static_cast<int64_t>(nDiffSeconds1) + static_cast<int64_t>(nDiffSeconds2);
	auto nOffsetMicroSeconds =  static_cast<int64_t>(nDiffMicroSeconds1) + static_cast<int64_t>(nDiffMicroSeconds2);

	const auto nOffsetSecondsAverage = static_cast<int32_t>(nOffsetSeconds / 2);
	const auto nOffsetMicrosAverage  = static_cast<int32_t>(nOffsetMicroSeconds / 2);

	ntp::time_t ntpOffset = {.tv_sec = nOffsetSecondsAverage, .tv_usec = nOffsetMicrosAverage};
	ntp::normalize_time(&ntpOffset);

	struct timeval tv;

	tv.tv_sec =	static_cast<time_t>(s_ntpClient.T4.nSeconds - ntp::JAN_1970) + nOffsetSecondsAverage;
	tv.tv_usec = (static_cast<int32_t>(USEC(s_ntpClient.T4.nFraction)) + static_cast<int32_t>(nOffsetMicrosAverage));

	settimeofday(&tv, nullptr);

	if ((ntpOffset.tv_sec == 0) && (ntpOffset.tv_usec > -999)  && (ntpOffset.tv_usec < 999)) {
		s_ntpClient.status = ntp::Status::LOCKED;
		ntpclient::display_status(ntp::Status::LOCKED);
		if (++s_ntpClient.nLockedCount == 4) {
			s_ntpClient.nPollSeconds = ntpclient::POLL_SECONDS_MAX;
		}
	} else {
		s_ntpClient.status = ntp::Status::IDLE;
		ntpclient::display_status(ntp::Status::IDLE);
		s_ntpClient.nPollSeconds = ntpclient::POLL_SECONDS_MIN;
		s_ntpClient.nLockedCount = 0;
	}

#ifndef NDEBUG
	const auto nTime = time(nullptr);
	const auto *pLocalTime = localtime(&nTime);
	DEBUG_PRINTF("localtime: %.4d/%.2d/%.2d %.2d:%.2d:%.2d", pLocalTime->tm_year + 1900, pLocalTime->tm_mon + 1, pLocalTime->tm_mday, pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);
#endif
	print_ntp_time("T1: ", &s_ntpClient.T1);
	print_ntp_time("T2: ", &s_ntpClient.T2);
	print_ntp_time("T3: ", &s_ntpClient.T3);
	print_ntp_time("T4: ", &s_ntpClient.T4);
#ifndef NDEBUG
	/* delay */
	ntp::time_t diff1;
	ntp::time_t diff2;

	difference(s_ntpClient.T1, s_ntpClient.T4, diff1.tv_sec, diff1.tv_usec);
	difference(s_ntpClient.T2, s_ntpClient.T3, diff2.tv_sec, diff2.tv_usec);

	ntp::time_t ntpDelay;
	ntp::sub_time(&ntpDelay, &diff1, &diff2);

	char sign = '+';

	if (ntpOffset.tv_sec < 0) {
		ntpOffset.tv_sec = -ntpOffset.tv_sec;
		sign = '-';
	}

	if (ntpOffset.tv_usec < 0) {
		ntpOffset.tv_usec = -ntpOffset.tv_usec;
		sign = '-';
	}

	printf(" offset=%c%d.%06d delay=%d.%06d\n", sign, ntpOffset.tv_sec, ntpOffset.tv_usec, ntpDelay.tv_sec, ntpDelay.tv_usec);
#endif
}

void input(const uint8_t *pBuffer, [[maybe_unused]] uint32_t nSize, [[maybe_unused]] uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	const auto *pReply = reinterpret_cast<const ntp::Packet *>(pBuffer);

	get_time_ntp_format(s_ntpClient.T4.nSeconds, s_ntpClient.T4.nFraction);

	if (__builtin_expect((nFromIp != s_ntpClient.nServerIp), 0)) {
		s_ntpClient.status = ntp::Status::FAILED;
		ntpclient::display_status(ntp::Status::FAILED);
		DEBUG_PUTS("ntp::Status::FAILED");
		return;
	}

	s_ntpClient.T2.nSeconds = __builtin_bswap32(pReply->ReceiveTimestamp_s);
	s_ntpClient.T2.nFraction = __builtin_bswap32(pReply->ReceiveTimestamp_f);

	s_ntpClient.T3.nSeconds = __builtin_bswap32(pReply->TransmitTimestamp_s);
	s_ntpClient.T3.nFraction = __builtin_bswap32(pReply->TransmitTimestamp_f);

	if (__builtin_expect(((pReply->LiVnMode & ntp::MODE_SERVER) == ntp::MODE_SERVER), 1)) {
		set_time_of_day();
	}
}

void ntp_client_init() {
	DEBUG_ENTRY

	memset(&s_ntpClient, 0, sizeof(struct ntpClient));

	s_ntpClient.nHandle = -1;

	s_ntpClient.Request.LiVnMode = ntp::VERSION | ntp::MODE_CLIENT;
	s_ntpClient.Request.Poll = ntpclient::POLL_POWER_MIN;
	s_ntpClient.Request.ReferenceID = ('A' << 0) | ('V' << 8) | ('S' << 16);

	NetworkParams networkParams;
	networkParams.Load();

	s_ntpClient.nServerIp = networkParams.GetNtpServer();

	if (s_ntpClient.nServerIp == 0) {
		s_ntpClient.status = ntp::Status::STOPPED;
	}

	DEBUG_EXIT
}

void ntp_client_start() {
	DEBUG_ENTRY

	if (s_ntpClient.status == ntp::Status::DISABLED) {
		DEBUG_EXIT
		return;
	}

	if (s_ntpClient.nServerIp == 0) {
		s_ntpClient.status = ntp::Status::STOPPED;
		ntpclient::display_status(ntp::Status::STOPPED);
		DEBUG_EXIT
		return;
	}

	s_ntpClient.nHandle = Network::Get()->Begin(ntp::UDP_PORT, input);
	assert(s_ntpClient.nHandle != -1);

	s_ntpClient.status = ntp::Status::IDLE;
	ntpclient::display_status(ntp::Status::IDLE);

	s_ntpClient.nTimerId = SoftwareTimerAdd(1000, ntp_client_timer);

	send();

	DEBUG_EXIT
}

void ntp_client_stop(const bool doDisable) {
	DEBUG_ENTRY

	if (doDisable) {
		s_ntpClient.status = ntp::Status::DISABLED;
		ntpclient::display_status(ntp::Status::DISABLED);
	}

	if (s_ntpClient.status == ntp::Status::STOPPED) {
		return;
	}

	SoftwareTimerDelete(s_ntpClient.nTimerId);

	Network::Get()->End(ntp::UDP_PORT);
	s_ntpClient.nHandle = -1;

	if (!doDisable) {
		s_ntpClient.status = ntp::Status::STOPPED;
		ntpclient::display_status(ntp::Status::STOPPED);
	}

	DEBUG_EXIT
}

void ntp_client_set_server_ip(const uint32_t nServerIp) {
	s_ntpClient.nServerIp = nServerIp;
}

uint32_t ntp_client_get_server_ip() {
	return s_ntpClient.nServerIp;
}

ntp::Status ntp_client_get_status() {
	return s_ntpClient.status;
}
