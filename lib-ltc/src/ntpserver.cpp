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

#if defined(DEBUG_NTPSERVER)
#undef NDEBUG
#endif

/*
 * https://tools.ietf.org/html/rfc5905
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <cassert>

#include "ntpserver.h"
#include "net/protocol/ntp.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

NtpServer::NtpServer(uint32_t year, uint32_t month, uint32_t day)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("year=%u, month=%u, day=%u", year, month, day);

    assert(s_this == nullptr);
    s_this = this;

    struct tm time_date;

    memset(&time_date, 0, sizeof(struct tm));
    time_date.tm_year = static_cast<int>(100 + year);
    time_date.tm_mon = static_cast<int>(month - 1);
    time_date.tm_mday = static_cast<int>(day);

    time_ = mktime(&time_date);
    assert(time_ != -1);

    DEBUG_PRINTF("time_=%.8x %ld", static_cast<unsigned int>(time_), time_);

    time_ += static_cast<time_t>(ntp::JAN_1970);

    DEBUG_PRINTF("time_=%.8x %ld", static_cast<unsigned int>(time_), time_);
    DEBUG_EXIT();
}

NtpServer::~NtpServer()
{
    Stop();
}

void NtpServer::Start()
{
    DEBUG_ENTRY();

    assert(handle_ == -1);
    handle_ = net::udp::Begin(ntp::UDP_PORT, StaticCallbackFunction);
    assert(handle_ != -1);

    reply_.LiVnMode = ntp::VERSION | ntp::MODE_SERVER;
    reply_.Stratum = ntp::STRATUM;
    reply_.Poll = ntp::MINPOLL;
    reply_.Precision = static_cast<uint8_t>(-10); // -9.9 = LOG2(0.0001) -> milliseconds
    reply_.RootDelay = 0;
    reply_.RootDispersion = 0;

    DEBUG_EXIT();
}

void NtpServer::Stop()
{
    DEBUG_ENTRY();

    assert(handle_ != -1);
    net::udp::End(ntp::UDP_PORT);
    handle_ = -1;

    DEBUG_EXIT();
}

void NtpServer::Print()
{
    printf("NTP v%d Server\n", ntp::VERSION >> 3);
    printf(" Port : %d\n", ntp::UDP_PORT);
    printf(" Stratum : %d\n", ntp::STRATUM);

    const auto kTime = static_cast<time_t>(static_cast<uint32_t>(time_) - ntp::JAN_1970);

    printf(" %s", asctime(localtime(&kTime)));
}
