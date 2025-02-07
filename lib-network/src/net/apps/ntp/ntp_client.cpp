/**
 * @file ntp_client.cpp
 * @brief Implementation of an NTP client for time synchronization in a standalone embedded environment.
 *
 * This file implements the functionality to synchronize system time using the Network Time Protocol (NTP).
 * It uses UDP to communicate with an NTP server and adjusts the system clock based on received timestamps.
 *
 * @note This implementation is optimized for Cortex-M3/M4/M7 processors and assumes a standalone environment
 *       without a standard library.
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
/**
 * @brief Macro to convert microseconds to NTP fraction.
 *
 * This is an optimized approximation for multiplying by 4294.967296 without
 * using floating point or 64-bit integers.
 */
#define NTPFRAC(x) ( 4294U*static_cast<uint32_t>(x) + ( (1981U*static_cast<uint32_t>(x))>>11 ) +  ((2911U*static_cast<uint32_t>(x))>>28) )

/*
 * The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
/**
 * @brief Macro to convert NTP fraction back to microseconds.
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

/**
 * @struct ntpClient
 * @brief Structure representing the state and configuration of the NTP client.
 */
struct ntpClient {
    uint32_t nServerIp;           ///< IP address of the NTP server.
    int32_t nHandle;              ///< Handle for UDP socket communication.
    TimerHandle_t nTimerId;       ///< Timer ID for periodic tasks.
    uint32_t nRequestTimeout;     ///< Timeout for NTP requests.
    uint32_t nPollSeconds;        ///< Polling interval in seconds.
    uint32_t nLockedCount;        ///< Counter for locked status.
    ntp::Status status;           ///< Current status of the NTP client.
    ntp::Packet Request;          ///< NTP request packet.
    uint8_t LiVnMode;             ///< Leap Indicator, Version, and Mode combined.
    ntp::TimeStamp T1;            ///< Originate timestamp.
    ntp::TimeStamp T2;            ///< Receive timestamp.
    ntp::TimeStamp T3;            ///< Transmit timestamp.
    ntp::TimeStamp T4;            ///< Destination timestamp.
};

// Local instance of the NTP client structure.
static ntpClient s_ntpClient;
static constexpr uint32_t REQUEST_SIZE = sizeof s_ntpClient.Request;

static void send();

/**
 * @brief Timer callback function for handling periodic tasks.
 *
 * This function manages polling intervals and request timeouts.
 *
 * @param nHandle Timer handle for the callback (unused).
 */
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

static struct timeval now;

/**
 * @brief Converts the current time to NTP format.
 *
 * This function retrieves the current system time and converts it to the
 * NTP timestamp format, including both seconds and fractional seconds.
 *
 * @param[out] nSeconds Number of seconds since 01/01/1900.
 * @param[out] nFraction Fractional part of a second in NTP format.
 */
static void get_time_ntp_format(uint32_t &nSeconds, uint32_t &nFraction) {
	gettimeofday(&now, nullptr);
	nSeconds = static_cast<uint32_t>(now.tv_sec) + ntp::JAN_1970;
	nFraction = NTPFRAC(now.tv_usec);
}

/**
 * @brief Sends an NTP request to the configured server.
 *
 * This function prepares an NTP request packet and sends it to the server
 * specified in the configuration using the UDP protocol.
 */
static void send() {
	get_time_ntp_format(s_ntpClient.T1.nSeconds, s_ntpClient.T1.nFraction);

	s_ntpClient.Request.TransmitTimestamp_s = __builtin_bswap32(s_ntpClient.T1.nSeconds);
	s_ntpClient.Request.TransmitTimestamp_f = __builtin_bswap32(s_ntpClient.T1.nFraction);

	Network::Get()->SendTo(s_ntpClient.nHandle, &s_ntpClient.Request, REQUEST_SIZE, s_ntpClient.nServerIp, ntp::UDP_PORT);

	s_ntpClient.nRequestTimeout = ntpclient::TIMEOUT_SECONDS;
	s_ntpClient.status = ntp::Status::WAITING;
	ntpclient::display_status(ntp::Status::WAITING);
}

/**
 * @brief Computes the time difference between two NTP timestamps.
 *
 * @param Start Start timestamp.
 * @param Stop Stop timestamp.
 * @param[out] nDiffSeconds Difference in seconds.
 * @param[out] nDiffMicroSeconds Difference in microseconds.
 */
static void difference(const struct ntp::TimeStamp& Start, const struct ntp::TimeStamp& Stop, int32_t &nDiffSeconds, int32_t &nDiffMicroSeconds) {
	ntp::time_t r;
	const ntp::time_t x = {.tv_sec = static_cast<int32_t>(Stop.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Stop.nFraction))};
	const ntp::time_t y = {.tv_sec = static_cast<int32_t>(Start.nSeconds), .tv_usec = static_cast<int32_t>(USEC(Start.nFraction))};
	ntp::sub_time(&r, &x, &y);

	nDiffSeconds = r.tv_sec;
	nDiffMicroSeconds = r.tv_usec;
}

/**
 * @brief Updates the system time based on NTP timestamps.
 *
 * This function calculates the time offset using NTP timestamps and adjusts
 * the system time accordingly.
 */
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

	tv.tv_sec =	now.tv_sec + nOffsetSecondsAverage;
	tv.tv_usec = now.tv_usec + nOffsetMicrosAverage;

	if (tv.tv_usec >= 1000000) {
	    tv.tv_sec += tv.tv_usec / 1000000;  // Add extra seconds
	    tv.tv_usec %= 1000000;              // Keep only the remainder microseconds
	} else if (tv.tv_usec < 0) {
	    tv.tv_sec -= 1 + (-tv.tv_usec / 1000000); // Subtract extra seconds
	    tv.tv_usec = 1000000 - (-tv.tv_usec % 1000000); // Adjust microseconds
	}

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

/**
 * @brief Processes an incoming NTP response.
 *
 * This function is called when an NTP response packet is received. It validates
 * the response, extracts the timestamps, and updates the system time if the
 * response is valid.
 *
 * @param[in] pBuffer Pointer to the buffer containing the NTP response packet.
 * @param[in] nSize Size of the received packet in bytes (unused).
 * @param[in] nFromIp IP address of the sender.
 * @param[in] nFromPort Port number of the sender (unused).
 *
 * @note This function verifies that the response is from the expected server and
 *       that it has a valid mode. If valid, it updates the system time using
 *       the extracted timestamps.
 */
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

/**
 * @brief Starts the NTP client.
 *
 * This function initializes the UDP socket for NTP communication, starts a software
 * timer for periodic tasks, and sends the first NTP request to the server.
 *
 * @note The function will not start the client if it is disabled or if the server
 *       IP address is not configured.
 */
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

/**
 * @brief Stops the NTP client.
 *
 * This function stops the software timer and closes the UDP socket. It optionally
 * disables the client if the `doDisable` parameter is set to `true`.
 *
 * @param[in] doDisable Set to `true` to disable the client after stopping.
 */
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

/**
 * @brief Sets the IP address of the NTP server.
 *
 * This function updates the server IP address used by the NTP client.
 *
 * @param[in] nServerIp The IP address of the NTP server.
 */
void ntp_client_set_server_ip(const uint32_t nServerIp) {
	s_ntpClient.nServerIp = nServerIp;
}

/**
 * @brief Retrieves the IP address of the configured NTP server.
 *
 * @return The IP address of the NTP server.
 */
uint32_t ntp_client_get_server_ip() {
	return s_ntpClient.nServerIp;
}

/**
 * @brief Retrieves the current status of the NTP client.
 *
 * This function returns the current operational status of the NTP client.
 *
 * @return The status of the NTP client as an `ntp::Status` enum.
 */
ntp::Status ntp_client_get_status() {
	return s_ntpClient.status;
}
