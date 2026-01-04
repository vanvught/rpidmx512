/**
 * @file ltcetc.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_LTCETC)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ltcetc.h"
#include "network.h"
#include "net/igmp.h"
#include "common/utils/utils_hex.h"
#include "firmware/debug/debug_dump.h"
#include "firmware/debug/debug_debug.h"

void LtcEtc::Start()
{
    if ((config_.destination_ip != 0) && (config_.destination_port != 0))
    {
        handle_.destination = net::udp::Begin(config_.destination_port, nullptr);
    }

    if (config_.source_port != 0)
    {
        handle_.source = net::udp::Begin(config_.source_port, StaticCallbackFunction);

        if ((handle_.source >= 0) && (config_.source_multicast_ip != 0))
        {
            net::igmp::JoinGroup(handle_.source, config_.source_multicast_ip);
        }
    }

    auto* p = s_sendbuffer;
    memcpy(p, Udp::kPrefix, sizeof(Udp::kPrefix));
    p += sizeof(Udp::kPrefix);
    memcpy(p, Udp::kHeader, sizeof(Udp::kHeader));
    p += sizeof(Udp::kHeader);
    p[2] = ' ';
    p[5] = ' ';
    p[8] = ' ';
    p[11] = ' ';
    p += sizeof(Udp::kTimecode);
    memcpy(p, Udp::kEnd, sizeof(Udp::kEnd));

    DEBUG_PRINTF("handle_.Destination=%d, handle_.Source=%d", handle_.destination, handle_.source);
}

void LtcEtc::Send(const midi::Timecode* time_code)
{
    if (handle_.destination < 0)
    {
        return;
    }

    assert(time_code != nullptr);

    auto* p = &s_sendbuffer[sizeof(Udp::kPrefix) + sizeof(Udp::kHeader)];

    const auto kData5 = static_cast<uint8_t>(((time_code->type) & 0x03) << 5) | (time_code->hours & 0x1F);

    *p++ = common::hex::ToCharUppercase(kData5 >> 4);
    *p = common::hex::ToCharUppercase(kData5 & 0x0F);
    p += 2;
    *p++ = common::hex::ToCharUppercase(time_code->minutes >> 4);
    *p = common::hex::ToCharUppercase(time_code->minutes & 0x0F);
    p += 2;
    *p++ = common::hex::ToCharUppercase(time_code->seconds >> 4);
    *p = common::hex::ToCharUppercase(time_code->seconds & 0x0F);
    p += 2;
    *p++ = common::hex::ToCharUppercase(time_code->frames >> 4);
    *p = common::hex::ToCharUppercase(time_code->frames & 0x0F);

    auto length = Udp::kMinMsgLength;

    switch (config_.terminator)
    {
        case ltcetc::UdpTerminator::kCr:
            s_sendbuffer[Udp::kMinMsgLength] = 0x0D;
            length++;
            break;
        case ltcetc::UdpTerminator::kLf:
            s_sendbuffer[Udp::kMinMsgLength] = 0x0A;
            length++;
            break;
        case ltcetc::UdpTerminator::kCrlf:
            s_sendbuffer[Udp::kMinMsgLength] = 0x0D;
            s_sendbuffer[Udp::kMinMsgLength + 1] = 0x0A;
            length = static_cast<uint16_t>(length + 2);
            break;
        default:
            break;
    }

    net::udp::Send(handle_.destination, reinterpret_cast<const uint8_t*>(s_sendbuffer), length, config_.destination_ip, config_.destination_port);

#ifndef NDEBUG
    debug::Dump(s_sendbuffer, length);
#endif
}

void LtcEtc::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    udp_buffer_ = reinterpret_cast<const char*>(buffer);
#ifndef NDEBUG
    debug::Dump(udp_buffer_, size);
#endif

    if (size == Udp::kMinMsgLength)
    {
        if (config_.terminator != ltcetc::UdpTerminator::kNone)
        {
            return;
        }
    }

    if (size == 1 + Udp::kMinMsgLength)
    {
        if (config_.terminator == ltcetc::UdpTerminator::kCrlf)
        {
            DEBUG_EXIT();
            return;
        }

        if ((config_.terminator == ltcetc::UdpTerminator::kCr) && (udp_buffer_[Udp::kMinMsgLength] != 0x0D))
        {
            DEBUG_EXIT();
            return;
        }

        if ((config_.terminator == ltcetc::UdpTerminator::kLf) && (udp_buffer_[Udp::kMinMsgLength] != 0x0A))
        {
            DEBUG_EXIT();
            return;
        }
    }

    if (size == 2 + Udp::kMinMsgLength)
    {
        if (config_.terminator != ltcetc::UdpTerminator::kCrlf)
        {
            DEBUG_EXIT();
            return;
        }

        if ((udp_buffer_[Udp::kMinMsgLength] != 0x0D) || (udp_buffer_[1 + Udp::kMinMsgLength] != 0x0A))
        {
            DEBUG_EXIT();
            ;
            return;
        }
    }

    if (memcmp(udp_buffer_, s_sendbuffer, sizeof(Udp::kPrefix) + sizeof(Udp::kHeader)) != 0)
    {
        DEBUG_EXIT();
        ;
        return;
    }

    ParseTimeCode();
}

static uint8_t FromHex(const char* hex)
{
    const auto kLow = (hex[1] > '9' ? (hex[1] | 0x20) - 'a' + 10 : hex[1] - '0');
    const auto kHigh = (hex[0] > '9' ? (hex[0] | 0x20) - 'a' + 10 : hex[0] - '0');
    return static_cast<uint8_t>((kHigh << 4) | kLow);
}

void LtcEtc::ParseTimeCode()
{
    if (handler_ != nullptr)
    {
        midi::Timecode time_code;

        auto* p = &udp_buffer_[sizeof(Udp::kPrefix) + sizeof(Udp::kHeader)];
        auto data = FromHex(p);

        time_code.hours = data & 0x1F;
        time_code.type = static_cast<uint8_t>(data >> 5);

        p += 3;

        time_code.minutes = FromHex(p);

        p += 3;

        time_code.seconds = FromHex(p);

        p += 3;

        time_code.frames = FromHex(p);

        DEBUG_PRINTF("%d:%d:%d:%d.%d", time_code.hours, time_code.minutes, time_code.seconds, time_code.frames, time_code.type);

        handler_->Handler(&time_code);
    }
}

void LtcEtc::Print()
{
    puts("ETC gateway");

    if ((config_.destination_ip != 0) && (config_.destination_port != 0))
    {
        printf(" Destination: " IPSTR ":%d\n", IP2STR(config_.destination_ip), config_.destination_port);
    }
    else
    {
        puts(" No output");
    }

    if (config_.source_port != 0)
    {
        printf("Source port: %d\n", config_.source_port);
        if (config_.source_multicast_ip != 0)
        {
            printf(" Multicast ip: " IPSTR, IP2STR(config_.source_multicast_ip));
        }
    }
    else
    {
        puts(" No input");
    }

    printf(" UDP Termination: %s\n", ltcetc::GetUdpTerminator(config_.terminator));
}
