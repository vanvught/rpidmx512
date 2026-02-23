/**
 * @file ntpclient.h
 *
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef APPS_NTPCLIENT_H_
#define APPS_NTPCLIENT_H_

#include <cstdint>

#include "core/protocol/ntp.h"

#if !defined(CONFIG_NTP_CLIENT_POLL_POWER_MIN)
#define CONFIG_NTP_CLIENT_POLL_POWER_MIN 3
#endif
#if !defined(CONFIG_NTP_CLIENT_POLL_POWER_MAX)
#define CONFIG_NTP_CLIENT_POLL_POWER_MAX 12
#endif

namespace network::apps::ntpclient
{
inline constexpr uint32_t kTimeoutSeconds = 3;
inline constexpr uint32_t kTimeoutMillis = kTimeoutSeconds * 1000;
inline constexpr uint8_t kPollPowerMin = CONFIG_NTP_CLIENT_POLL_POWER_MIN;
inline constexpr uint8_t kPollPowerMax = CONFIG_NTP_CLIENT_POLL_POWER_MAX;
inline constexpr uint32_t kPollSecondsMin = (1U << kPollPowerMin);
static_assert(kPollSecondsMin >= ntp::kMinpoll);
inline constexpr uint32_t kPollSecondsMax = (1U << kPollPowerMax);

void DisplayStatus(::ntp::Status status);

// Main NTP client interface
void Init();
void Start();
void Stop(bool do_disable = false);
void SetServerIp(uint32_t server_ip);
uint32_t GetServerIp();
ntp::Status GetStatus();

// PTP (GD32 only)
namespace ptp
{
void Init();
void Start();
void Stop(bool do_disable = false);
void SetServerIp(uint32_t server_ip);
uint32_t GetServerIp();
ntp::Status GetStatus();
} // namespace ptp

} // namespace network::apps::ntpclient

#endif // APPS_NTPCLIENT_H_
