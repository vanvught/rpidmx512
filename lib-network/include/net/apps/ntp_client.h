/**
 * @file ntp_client.h
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

#ifndef NET_APPS_NTP_CLIENT_H_
#define NET_APPS_NTP_CLIENT_H_

#include <cstdint>

#include "net/protocol/ntp.h"

#if !defined(CONFIG_NTP_CLIENT_POLL_POWER)
# define CONFIG_NTP_CLIENT_POLL_POWER 10
#endif

namespace ntpclient {
static constexpr uint32_t TIMEOUT_SECONDS = 3;
static constexpr uint32_t TIMEOUT_MILLIS = TIMEOUT_SECONDS * 1000;
static constexpr uint8_t POLL_POWER = CONFIG_NTP_CLIENT_POLL_POWER;
static constexpr uint32_t POLL_SECONDS = (1U << POLL_POWER);

void display_status(const ::ntp::Status status);
}  // namespace ntpclient

void ntp_client_init();
void ntp_client_start();
void ntp_client_stop(const bool doDisable = false);

void ntp_client_set_server_ip(const uint32_t nServerIp);
uint32_t ntp_client_get_server_ip();

#endif /* NET_APPS_NTP_CLIENT_H_ */
