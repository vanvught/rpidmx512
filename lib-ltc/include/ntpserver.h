/**
 * @file ntpserver.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NTPSERVER_H_
#define NTPSERVER_H_

#include <cstdint>
#include <cassert>
#include <time.h>

#include "ltc.h"
#include "network.h"
#include "core/protocol/ntp.h"

class NtpServer
{
   public:
    NtpServer(uint32_t year, uint32_t month, uint32_t day);
    ~NtpServer();

    void Start();
    void Stop();

    void SetTimeCode(const struct ltc::TimeCode* timecode)
    {
        time_date_ = time_;
        time_date_ += timecode->seconds;
        time_date_ += timecode->minutes * 60U;
        time_date_ += timecode->hours * 60U * 60U;

        const auto kType = static_cast<ltc::Type>(timecode->type);

        if (kType == ltc::Type::FILM)
        {
            fraction_ = static_cast<uint32_t>((178956970.625 * timecode->frames));
        }
        else if (kType == ltc::Type::EBU)
        {
            fraction_ = static_cast<uint32_t>((171798691.8 * timecode->frames));
        }
        else if ((kType == ltc::Type::DF) || (kType == ltc::Type::SMPTE))
        {
            fraction_ = static_cast<uint32_t>((143165576.5 * timecode->frames));
        }
        else
        {
            assert(0);
        }

        reply_.ReferenceTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(time_date_));
        reply_.ReferenceTimestamp_f = __builtin_bswap32(fraction_);
        reply_.ReceiveTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(time_date_));
        reply_.ReceiveTimestamp_f = __builtin_bswap32(fraction_);
        reply_.TransmitTimestamp_s = __builtin_bswap32(static_cast<uint32_t>(time_date_));
        reply_.TransmitTimestamp_f = __builtin_bswap32(fraction_);
    }

    /**
     * @brief Processes an incoming UDP packet.
     *
     * @param pBuffer Pointer to the packet buffer.
     * @param nSize Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        if (__builtin_expect((size != sizeof(struct ntp::Packet)), 0))
        {
            return;
        }

        auto* request = reinterpret_cast<const ntp::Packet*>(buffer);

        if (__builtin_expect(((request->LiVnMode & ntp::MODE_CLIENT) != ntp::MODE_CLIENT), 0))
        {
            return;
        }

        reply_.ReferenceID = network::GetPrimaryIp();
        reply_.OriginTimestamp_s = request->TransmitTimestamp_s;
        reply_.OriginTimestamp_f = request->TransmitTimestamp_f;

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&reply_), sizeof(struct ntp::Packet), from_ip, from_port);
    }

    void Print();

    static NtpServer* Get() { return s_this; }

   private:
    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param pBuffer Pointer to the packet buffer.
     * @param nSize Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    time_t time_{0};
    time_t time_date_{0};
    uint32_t fraction_{0};
    int32_t handle_{-1};
    ntp::Packet reply_;

    static inline NtpServer* s_this;
};

#endif  // NTPSERVER_H_
