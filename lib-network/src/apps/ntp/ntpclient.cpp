/**
 * @file ntpclient.cpp
 * @brief Implementation of an NTP client for time synchronization in a standalone embedded environment.
 *
 * This file implements the functionality to synchronize system time using the Network Time Protocol (NTP).
 * It uses UDP to communicate with an NTP server and adjusts the system clock based on received timestamps.
 *
 * @note This implementation is optimized for Cortex-M3/M4/M7 processors and assumes a standalone environment
 *       without a standard library.
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NTP_CLIENT)
#undef NDEBUG
#endif

#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cassert>
#include <ctime>
#include <sys/time.h>

#include "network.h"
#include "configstore.h"
#include "core/protocol/ntp.h"
#include "core/protocol/iana.h"
#include "apps/ntpclient.h"
#include "softwaretimers.h"
#include "configurationstore.h"
#include "firmware/debug/debug_debug.h"

namespace network::apps::ntpclient
{
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
#define NTPFRAC(x) (4294U * static_cast<uint32_t>(x) + ((1981U * static_cast<uint32_t>(x)) >> 11) + ((2911U * static_cast<uint32_t>(x)) >> 28))

/*
 * The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
/**
 * @brief Macro to convert NTP fraction back to microseconds.
 */
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

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
struct NtpClient
{
    uint32_t server_ip;       ///< IP address of the NTP server.
    int32_t handle;           ///< Handle for UDP socket communication.
    TimerHandle_t timer_id;   ///< Timer ID for periodic tasks.
    uint32_t request_timeout; ///< Timeout for NTP requests.
    uint32_t poll_seconds;    ///< Polling interval in seconds.
    uint32_t locked_count;    ///< Counter for locked status.
    ntp::Status status;       ///< Current status of the NTP client.
    ntp::Packet request;      ///< NTP request packet.
    uint8_t li_vn_mode;       ///< Leap Indicator, Version, and Mode combined.
    ntp::TimeStamp t1;        ///< Originate timestamp.
    ntp::TimeStamp t2;        ///< Receive timestamp.
    ntp::TimeStamp t3;        ///< Transmit timestamp.
    ntp::TimeStamp t4;        ///< Destination timestamp.
};

// Local instance of the NTP client structure.
static NtpClient s_ntp_client;
static constexpr uint32_t kRequestSize = sizeof s_ntp_client.request;

static void Send();

/**
 * @brief Timer callback function for handling periodic tasks.
 *
 * This function manages polling intervals and request timeouts.
 *
 * @param handle Timer handle for the callback (unused).
 */
static void NtpClientTimer([[maybe_unused]] TimerHandle_t handle)
{
    assert(s_ntp_client.status != ntp::Status::kStopped);
    assert(s_ntp_client.status != ntp::Status::kDisabled);

    if (s_ntp_client.status == ntp::Status::kWaiting)
    {
        if (s_ntp_client.request_timeout > 1)
        {
            s_ntp_client.request_timeout--;
            return;
        }

        if (s_ntp_client.request_timeout == 1)
        {
            s_ntp_client.status = ntp::Status::kFailed;
            ntpclient::DisplayStatus(ntp::Status::kFailed);
            s_ntp_client.poll_seconds = ntpclient::kPollSecondsMin;
            return;
        }
        return;
    }

    if (s_ntp_client.poll_seconds > 1)
    {
        s_ntp_client.poll_seconds--;
        return;
    }

    if (s_ntp_client.poll_seconds == 1)
    {
        Send();
    }
}

static void PrintNtpTime([[maybe_unused]] const char* text, [[maybe_unused]] const struct ntp::TimeStamp* ntp_time)
{
#ifndef NDEBUG
    const auto kSeconds = static_cast<time_t>(ntp_time->seconds - ntp::kJan1970);
    const auto* tm = localtime(&kSeconds);
    printf("%s %02d:%02d:%02d.%06d %04d [%u]\n", text, tm->tm_hour, tm->tm_min, tm->tm_sec, USEC(ntp_time->fraction), tm->tm_year + 1900, ntp_time->seconds);
#endif
}

static struct timeval now;

/**
 * @brief Converts the current time to NTP format.
 *
 * This function retrieves the current system time and converts it to the
 * NTP timestamp format, including both seconds and fractional seconds.
 *
 * @param[out] seconds Number of seconds since 01/01/1900.
 * @param[out] fraction Fractional part of a second in NTP format.
 */
static void GetTimeNtpFormat(uint32_t& seconds, uint32_t& fraction)
{
    gettimeofday(&now, nullptr);
    seconds = static_cast<uint32_t>(now.tv_sec) + ntp::kJan1970;
    fraction = NTPFRAC(now.tv_usec);
}

/**
 * @brief Sends an NTP request to the configured server.
 *
 */
static void Send()
{
    GetTimeNtpFormat(s_ntp_client.t1.seconds, s_ntp_client.t1.fraction);

    s_ntp_client.request.transmit_timestamp_s = __builtin_bswap32(s_ntp_client.t1.seconds);
    s_ntp_client.request.transmit_timestamp_f = __builtin_bswap32(s_ntp_client.t1.fraction);

    network::udp::Send(s_ntp_client.handle, reinterpret_cast<const uint8_t*>(&s_ntp_client.request), kRequestSize, s_ntp_client.server_ip, iana::Ports::kPortNtp);

    s_ntp_client.request_timeout = ntpclient::kTimeoutSeconds;
    s_ntp_client.status = ntp::Status::kWaiting;
    ntpclient::DisplayStatus(ntp::Status::kWaiting);
}

/**
 * @brief Computes the time difference between two NTP timestamps.
 *
 * @param Start Start timestamp.
 * @param Stop Stop timestamp.
 * @param[out] nDiffSeconds Difference in seconds.
 * @param[out] nDiffMicroSeconds Difference in microseconds.
 */
static void Difference(const struct ntp::TimeStamp& start, const struct ntp::TimeStamp& stop, int32_t& diff_seconds, int32_t& diff_micro_seconds)
{
    ntp::Time r;
    const ntp::Time kX = {.tv_sec = static_cast<int32_t>(stop.seconds), .tv_usec = static_cast<int32_t>(USEC(stop.fraction))};
    const ntp::Time kY = {.tv_sec = static_cast<int32_t>(start.seconds), .tv_usec = static_cast<int32_t>(USEC(start.fraction))};
    ntp::SubTime(&r, &kX, &kY);

    diff_seconds = r.tv_sec;
    diff_micro_seconds = r.tv_usec;
}

/**
 * @brief Updates the system time based on NTP timestamps.
 *
 * This function calculates the time offset using NTP timestamps and adjusts
 * the system time accordingly.
 */
static void SetTimeOfDay()
{
    int32_t diff_seconds1, diff_seconds2;
    int32_t diff_micro_seconds1, diff_micro_seconds2;

    Difference(s_ntp_client.t1, s_ntp_client.t2, diff_seconds1, diff_micro_seconds1);
    Difference(s_ntp_client.t4, s_ntp_client.t3, diff_seconds2, diff_micro_seconds2);

    auto offset_seconds = static_cast<int64_t>(diff_seconds1) + static_cast<int64_t>(diff_seconds2);
    auto offset_micro_seconds = static_cast<int64_t>(diff_micro_seconds1) + static_cast<int64_t>(diff_micro_seconds2);

    const auto kOffsetSecondsAverage = static_cast<int32_t>(offset_seconds / 2);
    const auto kOffsetMicrosAverage = static_cast<int32_t>(offset_micro_seconds / 2);

    ntp::Time ntp_offset = {.tv_sec = kOffsetSecondsAverage, .tv_usec = kOffsetMicrosAverage};
    ntp::NormalizeTime(&ntp_offset);

    struct timeval tv;

    tv.tv_sec = now.tv_sec + kOffsetSecondsAverage;
    tv.tv_usec = now.tv_usec + kOffsetMicrosAverage;

    if (tv.tv_usec >= 1000000)
    {
        tv.tv_sec += tv.tv_usec / 1000000; // Add extra seconds
        tv.tv_usec %= 1000000;             // Keep only the remainder microseconds
    }
    else if (tv.tv_usec < 0)
    {
        tv.tv_sec -= 1 + (-tv.tv_usec / 1000000);       // Subtract extra seconds
        tv.tv_usec = 1000000 - (-tv.tv_usec % 1000000); // Adjust microseconds
    }

    settimeofday(&tv, nullptr);

    if ((ntp_offset.tv_sec == 0) && (ntp_offset.tv_usec > -999) && (ntp_offset.tv_usec < 999))
    {
        s_ntp_client.status = ntp::Status::kLocked;
        ntpclient::DisplayStatus(ntp::Status::kLocked);
        if (++s_ntp_client.locked_count == 4)
        {
            s_ntp_client.poll_seconds = ntpclient::kPollSecondsMax;
        }
    }
    else
    {
        s_ntp_client.status = ntp::Status::kIdle;
        ntpclient::DisplayStatus(ntp::Status::kIdle);
        s_ntp_client.poll_seconds = ntpclient::kPollSecondsMin;
        s_ntp_client.locked_count = 0;
    }

#ifndef NDEBUG
    const auto kTime = time(nullptr);
    const auto* local_time = localtime(&kTime);
    DEBUG_PRINTF("localtime: %.4d/%.2d/%.2d %.2d:%.2d:%.2d", local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
#endif
    PrintNtpTime("T1: ", &s_ntp_client.t1);
    PrintNtpTime("T2: ", &s_ntp_client.t2);
    PrintNtpTime("T3: ", &s_ntp_client.t3);
    PrintNtpTime("T4: ", &s_ntp_client.t4);
#ifndef NDEBUG
    /* delay */
    ntp::Time diff1;
    ntp::Time diff2;

    Difference(s_ntp_client.t1, s_ntp_client.t4, diff1.tv_sec, diff1.tv_usec);
    Difference(s_ntp_client.t2, s_ntp_client.t3, diff2.tv_sec, diff2.tv_usec);

    ntp::Time ntp_delay;
    ntp::SubTime(&ntp_delay, &diff1, &diff2);

    char sign = '+';

    if (ntp_offset.tv_sec < 0)
    {
        ntp_offset.tv_sec = -ntp_offset.tv_sec;
        sign = '-';
    }

    if (ntp_offset.tv_usec < 0)
    {
        ntp_offset.tv_usec = -ntp_offset.tv_usec;
        sign = '-';
    }

    printf(" offset=%c%d.%06d delay=%d.%06d\n", sign, ntp_offset.tv_sec, ntp_offset.tv_usec, ntp_delay.tv_sec, ntp_delay.tv_usec);
#endif
}

/**
 * @brief Processes an incoming NTP response.
 *
 * This function is called when an NTP response packet is received. It validates
 * the response, extracts the timestamps, and updates the system time if the
 * response is valid.
 *
 * @param[in] buffer Pointer to the buffer containing the NTP response packet.
 * @param[in] size Size of the received packet in bytes (unused).
 * @param[in] from_ip IP address of the sender.
 * @param[in] from_port Port number of the sender (unused).
 *
 * @note This function verifies that the response is from the expected server and
 *       that it has a valid mode. If valid, it updates the system time using
 *       the extracted timestamps.
 */
void Input(const uint8_t* buffer, [[maybe_unused]] uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    const auto* reply = reinterpret_cast<const ntp::Packet*>(buffer);

    if (size < sizeof(ntp::Packet)) [[unlikely]]
    {
        return;
    }

    if (from_ip != s_ntp_client.server_ip) [[unlikely]]
    {
        return;
    }

    GetTimeNtpFormat(s_ntp_client.t4.seconds, s_ntp_client.t4.fraction);

    const uint8_t kLiVnMode = reply->li_vn_mode;
    const uint8_t kLi = (kLiVnMode >> 6) & 0x03;
    const uint8_t kVn = (kLiVnMode >> 3) & 0x07;
    const uint8_t kMode = (kLiVnMode >> 0) & 0x07;

    // Basic sanity: version 3 or 4, and mode "server"
    if (!((kVn == 3) || (kVn == 4))) [[unlikely]]
    {
        return;
    }

    if (kMode != 4) [[unlikely]]
    { // 4 = server
        return;
    }

    // Reject unsynchronized server
    if (kLi == 3) [[unlikely]]
    { // alarm condition
        return;
    }

    const uint8_t kStratum = reply->stratum;
    // 0 = KoD/special, 16 = unsynchronized (commonly)
    if ((kStratum == 0) || (kStratum >= 16)) [[unlikely]]
    {
        // TODO (a) Back off polling here.
        return;
    }

    // Origin timestamp must match our T1 (server echoes client's transmit time)
    const uint32_t kOrgS = __builtin_bswap32(reply->origin_timestamp_s);
    const uint32_t kOrgF = __builtin_bswap32(reply->origin_timestamp_f);

    if ((kOrgS != s_ntp_client.t1.seconds) || (kOrgF != s_ntp_client.t1.fraction)) [[unlikely]]
    {
        // Stale/unsolicited reply; ignore it.
        return;
    }

    s_ntp_client.t2.seconds = __builtin_bswap32(reply->receive_timestamp_s);
    s_ntp_client.t2.fraction = __builtin_bswap32(reply->receive_timestamp_f);

    s_ntp_client.t3.seconds = __builtin_bswap32(reply->transmit_timestamp_s);
    s_ntp_client.t3.fraction = __builtin_bswap32(reply->transmit_timestamp_f);

    SetTimeOfDay();
}

#pragma GCC pop_options
#pragma GCC push_options
#pragma GCC optimize("Os")

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
void Init()
{
    DEBUG_ENTRY();

    memset(&s_ntp_client, 0, sizeof(struct NtpClient));

    s_ntp_client.handle = -1;
    s_ntp_client.request.li_vn_mode = ntp::kVersion | ntp::kModeClient;
    s_ntp_client.request.poll = ntpclient::kPollPowerMin;
    s_ntp_client.request.reference_id = ('A' << 0) | ('V' << 8) | ('S' << 16);
    s_ntp_client.server_ip = ConfigStore::Instance().NetworkGet(&common::store::Network::ntp_server_ip);

    if (s_ntp_client.server_ip == 0)
    {
        s_ntp_client.status = ntp::Status::kStopped;
    }

    DEBUG_EXIT();
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
void Start()
{
    DEBUG_ENTRY();

    if (s_ntp_client.status == ntp::Status::kDisabled)
    {
        DEBUG_EXIT();
        return;
    }

    if (s_ntp_client.server_ip == 0)
    {
        s_ntp_client.status = ntp::Status::kStopped;
        ntpclient::DisplayStatus(ntp::Status::kStopped);
        DEBUG_EXIT();
        return;
    }

    s_ntp_client.handle = network::udp::Begin(iana::Ports::kPortNtp, Input);
    assert(s_ntp_client.handle != -1);

    s_ntp_client.status = ntp::Status::kIdle;
    ntpclient::DisplayStatus(ntp::Status::kIdle);

    s_ntp_client.timer_id = SoftwareTimerAdd(1000, NtpClientTimer);

    Send();

    DEBUG_EXIT();
}

/**
 * @brief Stops the NTP client.
 *
 * This function stops the software timer and closes the UDP socket. It optionally
 * disables the client if the `doDisable` parameter is set to `true`.
 *
 * @param[in] do_disable Set to `true` to disable the client after stopping.
 */
void Stop(bool do_disable)
{
    DEBUG_ENTRY();

    if (do_disable)
    {
        s_ntp_client.status = ntp::Status::kDisabled;
        ntpclient::DisplayStatus(ntp::Status::kDisabled);
    }

    if (s_ntp_client.status == ntp::Status::kStopped)
    {
        return;
    }

    SoftwareTimerDelete(s_ntp_client.timer_id);

    network::udp::End(iana::Ports::kPortNtp);
    s_ntp_client.handle = -1;

    if (!do_disable)
    {
        s_ntp_client.status = ntp::Status::kStopped;
        ntpclient::DisplayStatus(ntp::Status::kStopped);
    }

    DEBUG_EXIT();
}

/**
 * @brief Sets the IP address of the NTP server.
 *
 * This function updates the server IP address used by the NTP client.
 *
 * @param[in] server_ip The IP address of the NTP server.
 */
void SetServerIp(uint32_t server_ip)
{
    Stop();

    s_ntp_client.server_ip = server_ip;

    Start();
}

/**
 * @brief Retrieves the IP address of the configured NTP server.
 *
 * @return The IP address of the NTP server.
 */
uint32_t GetServerIp()
{
    return s_ntp_client.server_ip;
}

/**
 * @brief Retrieves the current status of the NTP client.
 *
 * This function returns the current operational status of the NTP client.
 *
 * @return The status of the NTP client as an `ntp::Status` enum.
 */
ntp::Status GetStatus()
{
    return s_ntp_client.status;
}
} // namespace network::apps::ntpclient