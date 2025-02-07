/**
 * @file net_ptp.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * https://www.ietf.org/archive/id/draft-ietf-ntp-interleaved-modes-07.html#name-interleaved-client-server-m
 */
/*
Server   t2   t3               t6   t7              t10  t11
    -----+----+----------------+----+----------------+----+-----
        /      \              /      \              /      \
Client /        \            /        \            /        \
    --+----------+----------+----------+----------+----------+--
      t1         t4         t5         t8         t9        t12

Mode: B         B           I         I           I         I
    +----+    +----+      +----+    +----+      +----+    +----+
Org | 0  |    | t1~|      | t2 |    | t4 |      | t6 |    | t8 |
Rx  | 0  |    | t2 |      | t4 |    | t6 |      | t8 |    |t10 |
Tx  | t1~|    | t3~|      | t1 |    | t3 |      | t5 |    | t7 |
    +----+    +----+      +----+    +----+      +----+    +----+

T1 - local transmit timestamp of the latest request (t5)
T2 - remote receive timestamp from the latest response (t6)
T3 - remote transmit timestamp from the latest response (t3)
T4 - local receive timestamp of the previous response (t4)
 */

#if defined (CONFIG_NET_ENABLE_NTP_CLIENT)
# error
#endif

#if defined (DEBUG_PTP_NTP_CLIENT)
# undef NDEBUG
#endif

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "hardware.h"
#include "network.h"
#include "networkparams.h"

#include "net/protocol/ntp.h"
#include "net/apps/ntp_client.h"

#include "gd32_ptp.h"

#include "softwaretimers.h"

#include "debug.h"

namespace ntpclient {
__attribute__((weak)) void display_status([[maybe_unused]] const ::ntp::Status status) {
	DEBUG_PRINTF("status=%u", static_cast<uint32_t>(status));
}
}  // namespace ntpclient


namespace net::globals {
	extern uint32_t ptpTimestamp[2];
} // namespace net::globals

#define _NTPFRAC_(x) ( 4294U*static_cast<uint32_t>(x) + ( (1981U*static_cast<uint32_t>(x))>>11 ) +  ((2911U*static_cast<uint32_t>(x))>>28) )
#define NTPFRAC(x)	_NTPFRAC_(x / 1000)

/**
 * The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

struct ntpClient {
	uint32_t nServerIp;
	int32_t nHandle;
	TimerHandle_t nTimerId;
	uint32_t nRequestTimeout;
	uint32_t nPollSeconds;
	uint32_t nLockedCount;
	const ntp::Packet *pReply;
	ntp::Status status;
	ntp::Packet Request;
	ntp::TimeStamp T1;	// time request sent by client
	ntp::TimeStamp T2;	// time request received by server
	ntp::TimeStamp T3;	// time reply sent by server
	ntp::TimeStamp T4;	// time reply received by client
	ntp::TimeStamp cookieBasic;
	struct {
		ntp::TimeStamp previousReceive;
		ntp::TimeStamp dst;	// destination timestamp
		ntp::TimeStamp sentA;
		ntp::TimeStamp sentB;
		int32_t	x;		// interleave switch
		uint32_t missedResponses;
#ifndef NDEBUG
		ntp::Modes mode;
#endif
	} state;
};

static ntpClient s_ntpClient;
static uint16_t s_id;
static constexpr uint16_t REQUEST_SIZE = sizeof s_ntpClient.Request;

static void print([[maybe_unused]] const char *pText, [[maybe_unused]] const struct ntp::TimeStamp *pNtpTime) {
#ifndef NDEBUG
	const auto nSeconds = static_cast<time_t>(pNtpTime->nSeconds - ntp::JAN_1970);
	const auto *pTm = localtime(&nSeconds);
	printf("%s %02d:%02d:%02d.%06d %04d [%u][0x%.8x]\n", pText, pTm->tm_hour, pTm->tm_min,  pTm->tm_sec, USEC(pNtpTime->nFraction), pTm->tm_year + 1900, pNtpTime->nSeconds, pNtpTime->nFraction);
#endif
}

static void process();

static void input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, [[maybe_unused]] uint16_t nFromPort) {
	DEBUG_ENTRY

	// Invalid packet size
	if (__builtin_expect((nSize != sizeof(struct ntp::Packet)), 1)) {
		DEBUG_EXIT
		return;
	}

	// Not for us
	if (__builtin_expect((nFromIp != s_ntpClient.nServerIp), 0)) {
		DEBUG_EXIT
		return;
	}

	// Ignore duplicates
	if (s_ntpClient.state.missedResponses == 0) {
		DEBUG_EXIT
		return;
	}

	s_ntpClient.pReply = reinterpret_cast<const ntp::Packet *>(pBuffer);

	process();

	DEBUG_EXIT
}

static void send();

static void ptp_ntp_timer([[maybe_unused]] TimerHandle_t nHandle) {
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

/**
 * The interleaved client/server mode is similar to the basic client/ server mode.
 * The difference between the two modes is in the values saved to the origin and transmit timestamp fields.
 */

static void send() {
	s_ntpClient.state.missedResponses++;

	/**
	 * The first request from a client is always in the basic mode and so is the server response.
	 * It has a zero origin timestamp and zero receive timestamp.
	 * Only when the client receives a valid response from the server,
	 * it will be able to send a request in the interleaved mode
	 */
	if (s_ntpClient.state.missedResponses > 4) {
		s_ntpClient.cookieBasic.nSeconds = random();
		s_ntpClient.cookieBasic.nFraction = 0;

		s_ntpClient.Request.OriginTimestamp_s = 0;
		s_ntpClient.Request.OriginTimestamp_f = 0;

		s_ntpClient.Request.ReceiveTimestamp_s = 0;
		s_ntpClient.Request.ReceiveTimestamp_f = 0;

		/**
		 * The origin timestamp is a cookie which is used to detect that a received packet
		 * is a response to the last packet sent in the other direction of the association.
		 */
		s_ntpClient.Request.TransmitTimestamp_s = __builtin_bswap32(s_ntpClient.cookieBasic.nSeconds);
		s_ntpClient.Request.TransmitTimestamp_f = __builtin_bswap32(s_ntpClient.cookieBasic.nFraction);
	} else {
		/**
		 * A client request in the interleaved mode has an origin timestamp equal to
		 *  the receive timestamp from the last valid server response.
		 */
		s_ntpClient.Request.OriginTimestamp_s = __builtin_bswap32(s_ntpClient.state.dst.nSeconds);
		s_ntpClient.Request.OriginTimestamp_f = __builtin_bswap32(s_ntpClient.state.dst.nFraction);

		s_ntpClient.Request.ReceiveTimestamp_s = __builtin_bswap32(s_ntpClient.state.previousReceive.nSeconds);
		s_ntpClient.Request.ReceiveTimestamp_f = __builtin_bswap32(s_ntpClient.state.previousReceive.nFraction);

		if (s_ntpClient.state.x > 0) {
			s_ntpClient.Request.TransmitTimestamp_s = __builtin_bswap32(s_ntpClient.state.sentB.nSeconds);
			s_ntpClient.Request.TransmitTimestamp_f = __builtin_bswap32(s_ntpClient.state.sentB.nFraction);
		} else {
			s_ntpClient.Request.TransmitTimestamp_s = __builtin_bswap32(s_ntpClient.state.sentA.nSeconds);
			s_ntpClient.Request.TransmitTimestamp_f = __builtin_bswap32(s_ntpClient.state.sentA.nFraction);
		}
	}

	Network::Get()->SendToTimestamp(s_ntpClient.nHandle, &s_ntpClient.Request, REQUEST_SIZE, s_ntpClient.nServerIp, ntp::UDP_PORT);

#ifndef NDEBUG
	printf("Request:  org=%.8x%.8x rx=%.8x%.8x tx=%.8x%.8x\n",
			__builtin_bswap32(s_ntpClient.Request.OriginTimestamp_s),  __builtin_bswap32(s_ntpClient.Request.OriginTimestamp_f),
			__builtin_bswap32(s_ntpClient.Request.ReceiveTimestamp_s),  __builtin_bswap32(s_ntpClient.Request.ReceiveTimestamp_f),
			__builtin_bswap32(s_ntpClient.Request.TransmitTimestamp_s),  __builtin_bswap32(s_ntpClient.Request.TransmitTimestamp_f));
#endif

	if (s_ntpClient.state.x > 0) {
		s_ntpClient.state.sentA.nSeconds = net::globals::ptpTimestamp[1] + ntp::JAN_1970;
		s_ntpClient.state.sentA.nFraction = NTPFRAC(gd32::ptp_subsecond_2_nanosecond(net::globals::ptpTimestamp[0]));
	} else {
		s_ntpClient.state.sentB.nSeconds = net::globals::ptpTimestamp[1] + ntp::JAN_1970;
		s_ntpClient.state.sentB.nFraction = NTPFRAC(gd32::ptp_subsecond_2_nanosecond(net::globals::ptpTimestamp[0]));
	}

	s_ntpClient.state.x = -s_ntpClient.state.x;
	s_id++;

	s_ntpClient.nRequestTimeout = ntpclient::TIMEOUT_SECONDS;
	s_ntpClient.status = ntp::Status::WAITING;
	ntpclient::display_status(ntp::Status::WAITING);
}

static void difference(const ntp::TimeStamp& Start, const ntp::TimeStamp& Stop, int32_t& nDiffSeconds, int32_t& nDiffNanoSeconds) {
	gd32::ptp::time_t r;
	const gd32::ptp::time_t x = {.tv_sec = static_cast<int32_t>(Stop.nSeconds), .tv_nsec = static_cast<int32_t>(USEC(Stop.nFraction) * 1000)};
	const gd32::ptp::time_t y = {.tv_sec = static_cast<int32_t>(Start.nSeconds), .tv_nsec = static_cast<int32_t>(USEC(Start.nFraction) * 1000)};
	gd32::sub_time(&r, &x, &y);

	nDiffSeconds = r.tv_sec;
	nDiffNanoSeconds = r.tv_nsec;
}

static inline int32_t abs_int32(const int32_t x) {
    return (x < 0) ? -x : x;
}

static void update_ptp_time() {
	int32_t nDiffSeconds1, nDiffNanoSeconds1;
	difference(s_ntpClient.T1, s_ntpClient.T2, nDiffSeconds1, nDiffNanoSeconds1);

	int32_t nDiffSeconds2, nDiffNanoSeconds2;
	difference(s_ntpClient.T4, s_ntpClient.T3, nDiffSeconds2, nDiffNanoSeconds2);

	auto nOffsetSeconds = static_cast<int64_t>(nDiffSeconds1) + static_cast<int64_t>(nDiffSeconds2);
	auto nOffsetNanoSeconds = static_cast<int64_t>(nDiffNanoSeconds1) + static_cast<int64_t>(nDiffNanoSeconds2);

	const int32_t nOffsetSecondsAverage = nOffsetSeconds / 2;
	const int32_t nOffsetNanosverage  = nOffsetNanoSeconds / 2;

	gd32::ptp::time_t ptpOffset = {.tv_sec = nOffsetSecondsAverage, .tv_nsec = nOffsetNanosverage};
	gd32::normalize_time(&ptpOffset);
	gd32_ptp_update_time(&ptpOffset);

	gd32::ptp::ptptime ptp_get;
	gd32_ptp_get_time(&ptp_get);

	const auto offset_ns = (static_cast<int64_t>(ptpOffset.tv_sec) * 1000000000LL) + static_cast<int64_t>(ptpOffset.tv_nsec);
	int32_t adjust_ppb = -(offset_ns / s_ntpClient.nPollSeconds);

	if (abs_int32(adjust_ppb) > 1) {
		gd32_adj_frequency(adjust_ppb);
#ifndef NDEBUG
		printf("Applied frequency adjustment: %d ppb\n", adjust_ppb);
#endif
	}

	s_ntpClient.Request.ReferenceTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(ptp_get.tv_sec) + ntp::JAN_1970);
	s_ntpClient.Request.ReferenceTimestamp_f  = __builtin_bswap32(NTPFRAC(ptp_get.tv_nsec));

	if ((ptpOffset.tv_sec == 0) && (ptpOffset.tv_nsec > -999999)  && (ptpOffset.tv_nsec < 999999)) {
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
	/**
	 * Network delay calculation
	 */
	gd32::ptp::time_t diff1;
	gd32::ptp::time_t diff2;

	if (s_ntpClient.state.mode == ntp::Modes::BASIC) {
		difference(s_ntpClient.T1, s_ntpClient.T4, diff1.tv_sec, diff1.tv_nsec);
		difference(s_ntpClient.T2, s_ntpClient.T3, diff2.tv_sec, diff2.tv_nsec);
	} else {
		ntp::TimeStamp start;
		start.nSeconds = __builtin_bswap32(s_ntpClient.Request.TransmitTimestamp_s);
		start.nFraction = __builtin_bswap32(s_ntpClient.Request.TransmitTimestamp_f);
		ntp::TimeStamp stop;
		stop.nSeconds = __builtin_bswap32(s_ntpClient.Request.ReceiveTimestamp_s);
		stop.nFraction = __builtin_bswap32(s_ntpClient.Request.ReceiveTimestamp_f);
		difference(start, stop, diff1.tv_sec, diff1.tv_nsec);

		const auto *const pReply = s_ntpClient.pReply;
		start.nSeconds = __builtin_bswap32(s_ntpClient.Request.OriginTimestamp_s);
		start.nFraction = __builtin_bswap32(s_ntpClient.Request.OriginTimestamp_f);
		stop.nSeconds = __builtin_bswap32(pReply->TransmitTimestamp_s);
		stop.nFraction = __builtin_bswap32(pReply->TransmitTimestamp_f);
		difference(start, stop, diff2.tv_sec, diff2.tv_nsec);
	}

	gd32::ptp::time_t ptpDelay;
	gd32::sub_time(&ptpDelay, &diff1, &diff2);

	char sign = '+';

	if (ptpOffset.tv_sec < 0) {
		ptpOffset.tv_sec = -ptpOffset.tv_sec;
		sign = '-';
	}

	if (ptpOffset.tv_nsec < 0) {
		ptpOffset.tv_nsec = -ptpOffset.tv_nsec;
		sign = '-';
	}

	printf(" %s : offset=%c%d.%09d delay=%d.%09d\n",
			s_ntpClient.state.mode == ntp::Modes::BASIC ? "Basic" : "Interleaved",
					sign, ptpOffset.tv_sec, ptpOffset.tv_nsec,
					ptpDelay.tv_sec, ptpDelay.tv_nsec);
#endif
}

/**
 * Two of the tests are modified for the interleaved mode:
 *
 * 1. The check for duplicate packets SHOULD compare both receive and
 *    transmit timestamps in order to not drop a valid response in the interleaved mode if
 *    it follows a response in the basic mode and they contain the same transmit timestamp.
 * 2. The check for bogus packets SHOULD compare the origin timestamp with both transmit and
 *    receive timestamps from the request.
 *    If the origin timestamp is equal to the transmit timestamp, the response is in the basic mode.
 *    If the origin timestamp is equal to the receive timestamp, the response is in the interleaved mode.
 */

static void process() {
	const auto *const pReply = s_ntpClient.pReply;
#ifndef NDEBUG
	printf("Response: org=%.8x%.8x rx=%.8x%.8x tx=%.8x%.8x\n",
			__builtin_bswap32(pReply->OriginTimestamp_s),  __builtin_bswap32(pReply->OriginTimestamp_f),
			__builtin_bswap32(pReply->ReceiveTimestamp_s),  __builtin_bswap32(pReply->ReceiveTimestamp_f),
			__builtin_bswap32(pReply->TransmitTimestamp_s),  __builtin_bswap32(pReply->TransmitTimestamp_f));
#endif
	/**
	 * If the origin timestamp is equal to the transmit timestamp,
	 * the response is in the basic mode.
	 */
	if ((pReply->OriginTimestamp_s == s_ntpClient.Request.TransmitTimestamp_s) && (pReply->OriginTimestamp_f == s_ntpClient.Request.TransmitTimestamp_f)) {
		if (s_ntpClient.state.x < 0) {
			s_ntpClient.T1.nSeconds = s_ntpClient.state.sentA.nSeconds;
			s_ntpClient.T1.nFraction = s_ntpClient.state.sentA.nFraction;
		} else {
			s_ntpClient.T1.nSeconds = s_ntpClient.state.sentB.nSeconds;
			s_ntpClient.T1.nFraction = s_ntpClient.state.sentB.nFraction;
		}

		s_ntpClient.T4.nSeconds = net::globals::ptpTimestamp[1] + ntp::JAN_1970;
		s_ntpClient.T4.nFraction = NTPFRAC(gd32::ptp_subsecond_2_nanosecond(net::globals::ptpTimestamp[0]));
#ifndef NDEBUG
		s_ntpClient.state.mode = ntp::Modes::BASIC;
#endif
	} else
		/**
		 * If the origin timestamp is equal to the receive timestamp,
		 * the response is in the interleaved mode.
		 */
		if ((pReply->OriginTimestamp_s == s_ntpClient.Request.ReceiveTimestamp_s) && (pReply->OriginTimestamp_f == s_ntpClient.Request.ReceiveTimestamp_f)) {
			if (s_ntpClient.state.x > 0) {
				s_ntpClient.T1.nSeconds = s_ntpClient.state.sentB.nSeconds;
				s_ntpClient.T1.nFraction = s_ntpClient.state.sentB.nFraction;
			} else {
				s_ntpClient.T1.nSeconds = s_ntpClient.state.sentA.nSeconds;
				s_ntpClient.T1.nFraction = s_ntpClient.state.sentA.nFraction;
			}

			s_ntpClient.T4.nSeconds = s_ntpClient.state.previousReceive.nSeconds;
			s_ntpClient.T4.nFraction = s_ntpClient.state.previousReceive.nFraction;
#ifndef NDEBUG
			s_ntpClient.state.mode = ntp::Modes::INTERLEAVED;
#endif
		} else {
			DEBUG_PUTS("INVALID RESPONSE");
			return;
		}

	s_ntpClient.T2.nSeconds = __builtin_bswap32(pReply->ReceiveTimestamp_s);
	s_ntpClient.T2.nFraction = __builtin_bswap32(pReply->ReceiveTimestamp_f);

	s_ntpClient.T3.nSeconds = __builtin_bswap32(pReply->TransmitTimestamp_s);
	s_ntpClient.T3.nFraction = __builtin_bswap32(pReply->TransmitTimestamp_f);

	s_ntpClient.state.dst.nSeconds = __builtin_bswap32(pReply->ReceiveTimestamp_s);
	s_ntpClient.state.dst.nFraction = __builtin_bswap32(pReply->ReceiveTimestamp_f);

	s_ntpClient.state.previousReceive.nSeconds = net::globals::ptpTimestamp[1] + ntp::JAN_1970;
	s_ntpClient.state.previousReceive.nFraction = NTPFRAC(gd32::ptp_subsecond_2_nanosecond(net::globals::ptpTimestamp[0]));

	update_ptp_time();

	s_ntpClient.state.missedResponses = 0;

	print("T1: ", &s_ntpClient.T1);
	print("T2: ", &s_ntpClient.T2);
	print("T3: ", &s_ntpClient.T3);
	print("T4: ", &s_ntpClient.T4);
}

/**
 * @brief Initializes the Precision Time Protocol (PTP) NTP client.
 *
 * This function performs the initial setup for the NTP client, including
 * clearing its state, setting default values, and loading network parameters
 * such as the NTP server's IP address.
 *
 * The initialization includes:
 * - Resetting all state variables to their default values.
 * - Configuring the initial NTP request packet parameters.
 * - Setting the client status to `IDLE`.
 * - Initializing the random number generator using the current system time.
 * - Loading network parameters to retrieve the configured NTP server IP address.
 *
 * @note This function must be called before starting the NTP client.
 */
void ptp_ntp_init() {
	DEBUG_ENTRY

	memset(&s_ntpClient, 0, sizeof(struct ntpClient));

	s_ntpClient.state.previousReceive.nSeconds = ntp::JAN_1970;
	s_ntpClient.state.dst.nSeconds = ntp::JAN_1970;
	s_ntpClient.state.sentA.nSeconds = ntp::JAN_1970;
	s_ntpClient.state.sentB.nSeconds = ntp::JAN_1970;
	s_ntpClient.state.missedResponses = 4;

	s_ntpClient.Request.LiVnMode = ntp::VERSION | ntp::MODE_CLIENT;
	s_ntpClient.Request.Poll = ntpclient::POLL_POWER_MIN;
	s_ntpClient.Request.ReferenceID = ('A' << 0) | ('V' << 8) | ('S' << 16);

	s_ntpClient.state.x = 1;
	s_ntpClient.status = ntp::Status::IDLE;

	struct timeval tv;
	gettimeofday(&tv, nullptr);
	srandom(static_cast<unsigned int>(tv.tv_sec ^ tv.tv_usec));

	NetworkParams networkParams;
	networkParams.Load();

	s_ntpClient.nServerIp = networkParams.GetNtpServer();

	DEBUG_EXIT
}

/**
 * @brief Starts the NTP client.
 *
 * This function initializes the UDP socket for NTP communication, starts a software
 * timer for periodic tasks, and sends the first NTP request to the server.
 *
 * @note The function will not start the client if it is disabled or if the server
 *       IP address is not configured.
 */
void ptp_ntp_start() {
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

	s_ntpClient.nTimerId = SoftwareTimerAdd(1000, ptp_ntp_timer);

	send();

	DEBUG_EXIT
}

/**
 * @brief Stops the NTP client.
 *
 * This function stops the software timer and closes the UDP socket. It optionally
 * disables the client if the `doDisable` parameter is set to `true`.
 *
 * @param[in] doDisable Set to `true` to disable the client after stopping.
 */
void ptp_ntp_stop(const bool doDisable) {
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

/**
 * @brief Sets the IP address of the NTP server.
 *
 * This function updates the server IP address used by the NTP client.
 *
 * @param[in] nServerIp The IP address of the NTP server.
 */
void ptp_ntp_set_server_ip(const uint32_t nServerIp) {
	s_ntpClient.nServerIp = nServerIp;
}

/**
 * @brief Retrieves the IP address of the configured NTP server.
 *
 * @return The IP address of the NTP server.
 */
uint32_t ptp_ntp_get_server_ip() {
	return s_ntpClient.nServerIp;
}

/**
 * @brief Retrieves the current status of the NTP client.
 *
 * This function returns the current operational status of the NTP client.
 *
 * @return The status of the NTP client as an `ntp::Status` enum.
 */
ntp::Status ptp_ntp_get_status() {
	return s_ntpClient.status;
}
