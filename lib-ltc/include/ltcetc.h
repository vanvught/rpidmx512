/**
 * @file ltcetc.h
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

#ifndef LTCETC_H_
#define LTCETC_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "common/utils/utils_enum.h"
#include "midi.h"
#include "ip4/ip4_address.h"

namespace ltcetc
{
enum class UdpTerminator : uint8_t
{
    kNone,
    kCr,
    kLf,
    kCrlf,
    kUndefined
};

inline constexpr uint32_t kMaxNameLength = 5;
inline constexpr const char kUdpTerminator[static_cast<uint32_t>(ltcetc::UdpTerminator::kUndefined)][kMaxNameLength] = {"None", "CR", "LF", "CRLF"};

[[nodiscard]] inline constexpr const char* GetUdpTerminator(UdpTerminator upd_terminator)
{
    return upd_terminator < UdpTerminator::kUndefined ? kUdpTerminator[static_cast<uint32_t>(upd_terminator)] : "UNDEFINED";
}

inline UdpTerminator GetUdpTerminator(const char* string)
{
    {
        assert(string != nullptr);
        uint8_t index = 0;

        for (const char(&module)[kMaxNameLength] : kUdpTerminator)
        {
            if (strcasecmp(string, module) == 0)
            {
                return common::FromValue<UdpTerminator>(index);
            }
            ++index;
        }

        return UdpTerminator::kUndefined;
    }
}
} // namespace ltcetc

class LtcEtcHandler
{
   public:
    virtual ~LtcEtcHandler() = default;

    virtual void Handler(const midi::Timecode* timecode) = 0;
};

class LtcEtc
{
   public:
    LtcEtc()
    {
        assert(s_this == nullptr);
        s_this = this;
    }

    void SetDestinationIp(uint32_t destination_ip)
    {
        if ((network::IsPrivateIp(destination_ip) || network::IsMulticastIp(destination_ip)))
        {
            config_.destination_ip = destination_ip;
        }
        else
        {
            config_.destination_ip = 0;
        }
    }

    uint32_t GetDestinationIp() const { return config_.destination_ip; }

    void SetDestinationPort(uint16_t destination_port)
    {
        if (destination_port > 1023)
        {
            config_.destination_port = destination_port;
        }
        else
        {
            config_.destination_port = 0;
        }
    }

    uint16_t GetDestinationPort() const { return config_.destination_port; }

    void SetSourceMulticastIp(uint32_t source_multicast_ip)
    {
        if (network::IsMulticastIp(source_multicast_ip))
        {
            config_.source_multicast_ip = source_multicast_ip;
        }
        else
        {
            config_.source_multicast_ip = 0;
        }
    }

    uint32_t GetSourceMulticastIp() const { return config_.source_multicast_ip; }

    void SetSourcePort(uint16_t source_port)
    {
        if (source_port > 1023)
        {
            config_.source_port = source_port;
        }
        else
        {
            config_.source_port = 0;
        }
    }

    uint16_t GetSourcePort() const { return config_.source_port; }

    void SetUdpTerminator(ltcetc::UdpTerminator terminator)
    {
        if (terminator < ltcetc::UdpTerminator::kUndefined)
        {
            config_.terminator = terminator;
        }
    }

    ltcetc::UdpTerminator GetUdpTerminator() const { return config_.terminator; }

    void SetHandler(LtcEtcHandler* handler) { handler_ = handler; }

    void Start();

    void Send(const midi::Timecode* timecode);

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

	void Print();

    static LtcEtc* Get() { return s_this; }

   private:
    void ParseTimeCode();

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    const char* udp_buffer_{nullptr};

    struct Config
    {
        uint32_t destination_ip;
        uint32_t source_multicast_ip;
        uint16_t destination_port;
        uint16_t source_port;
        ltcetc::UdpTerminator terminator;
    };

    Config config_ = {0, 0, 0, 0, ltcetc::UdpTerminator::kNone};

    struct Handle
    {
        int destination;
        int source;
    };

    Handle handle_{-1, -1};

    LtcEtcHandler* handler_{nullptr};

    struct Udp
    {
        static constexpr char kPrefix[] = {'M', 'I', 'D', 'I', ' '};
        static constexpr char kHeader[] = {'F', '0', ' ', '7', 'F', ' ', '7', 'F', ' ', '0', '1', ' ', '0', '1', ' '};
        static constexpr char kTimecode[] = {'H', 'H', ' ', 'M', 'M', ' ', 'S', 'S', ' ', 'F', 'F', ' '};
        static constexpr char kEnd[] = {'F', '7'};
        static constexpr auto kMinMsgLength = sizeof(kPrefix) + sizeof(kHeader) + sizeof(kTimecode) + sizeof(kEnd);
    };

    static inline char s_sendbuffer[Udp::kMinMsgLength + 2];
    static inline LtcEtc* s_this;
};

#endif // LTCETC_H_
