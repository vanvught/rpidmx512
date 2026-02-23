/**
 * @file dns.h
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_PROTOCOL_DNS_H_
#define CORE_PROTOCOL_DNS_H_

#include <cstdint>

#include "ip4/ip4_address.h"

namespace network::dns
{
inline constexpr uint32_t kSizeofDnsHdr = 12;

enum class Flag1 : uint8_t
{
    kResponse = 0x80,       ///< query (0), or a response (1).
    kOpcodeStatus = 0x10,   ///< a server status request (STATUS)
    kOpcodeIquery = 0x08,   ///< an inverse query (IQUERY)
    kOpcodeStandard = 0x00, ///< (RFC 6762, section 18.3)
    kAuthorative = 0x04,    ///< Authoritative Answer
    kTrunc = 0x02,          ///< TrunCation
    kRd = 0x01              ///< If RD is set, it directs the name server to pursue the query recursively.
};

static constexpr uint8_t operator|(Flag1 a, Flag1 b)
{
    return static_cast<uint8_t>((static_cast<uint8_t>(a) | static_cast<uint8_t>(b)));
}

///< NS field TYPE used for "Resource Records"
enum class RRType : uint16_t
{
    kA = 1,    ///< a host address
    kPtr = 12, ///< a domain name pointer
    kTxt = 16, ///< text strings
    kSrv = 33, ///< service location
    kAll = 255 ///< any type
};

///< DNS field CLASS used for "Resource Records"
struct RRClass
{
    static constexpr uint16_t kInternet = 1;   ///< Internet
    static constexpr uint16_t kAny = 255;      ///< Any class
    static constexpr uint16_t kFlush = 0x8000; ///< Flush bit
};

struct Header
{
    uint16_t xid;
    uint8_t flag1;
    uint8_t flag2;
    uint16_t query_count;
    uint16_t answer_count;
    uint16_t authority_count;
    uint16_t additional_count;
} __attribute__((__packed__));

inline uint8_t DnsHeaderGetOpcode(const Header* const kHeader)
{
    return ((kHeader->flag1) >> 3) & 0xF;
}

// mDNS
inline constexpr uint32_t kMulticastMessageSize = 512; ///< The 1987 DNS specification [RFC1035] restricts DNS messages carried by UDP to no more than 512 bytes
inline constexpr uint32_t kMulticastAddress = network::ConvertToUint(224, 0, 0, 251);
} // namespace network::dns

#endif /* CORE_PROTOCOL_DNS_H_ */
