/**
 * @file dhcp.h
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

#ifndef CORE_PROTOCOL_DHCP_H_
#define CORE_PROTOCOL_DHCP_H_

#include <cstdint>

#include "core/protocol/ip4.h"

namespace network::dhcp
{
inline constexpr uint32_t kOptSize = 312;
inline constexpr uint32_t kMagicCookie = 0x63825363; ///< You should not modify it number.

struct OpCode
{
    static constexpr uint8_t kBootrequest = 1;
    static constexpr uint8_t kBootreply = 2;
};

struct HardwareType
{
    static constexpr uint8_t k10Mb = 1;
    static constexpr uint8_t k100Mb = 2;
};

struct Type
{
    static constexpr uint8_t kDiscover = 1;
    static constexpr uint8_t kOffer = 2;
    static constexpr uint8_t kRequest = 3;
    static constexpr uint8_t kDecline = 4;
    static constexpr uint8_t kAck = 5;
    static constexpr uint8_t kNak = 6;
    static constexpr uint8_t kRelease = 7;
    static constexpr uint8_t kInform = 8;
};

struct Options
{
    // BootP options
    static constexpr uint8_t kPadOption = 0;
    static constexpr uint8_t kSubnetMask = 1; ///< RFC 2132 3.3
    static constexpr uint8_t kRouter = 3;
    static constexpr uint8_t kDnsServer = 6;
    static constexpr uint8_t kHostname = 12;
    static constexpr uint8_t kDomainName = 15;
    static constexpr uint8_t kIpTtl = 23;
    static constexpr uint8_t kMtu = 26;
    static constexpr uint8_t kBroadcast = 28;
    static constexpr uint8_t kTcpTtl = 37;
    static constexpr uint8_t kNtp = 42;
    static constexpr uint8_t kEnd = 255;
    // DHCP options
    static constexpr uint8_t kRequestedIp = 50;      ///< RFC 2132 9.1, requested IP address
    static constexpr uint8_t kLeaseTime = 51;        ///< RFC 2132 9.2, time in seconds, in 4 bytes
    static constexpr uint8_t kOverload = 52;         ///< RFC2132 9.3, use file and/or sname field for options
    static constexpr uint8_t kMessageType = 53;      ///< RFC 2132 9.6, important for DHCP
    static constexpr uint8_t kServerIdentifier = 54; ///< RFC 2132 9.7, server IP address
    static constexpr uint8_t kParamRequest = 55;     ///< RFC 2132 9.8, requested option types
    static constexpr uint8_t kMaxMsgSize = 57;       ///< RFC 2132 9.10, message size accepted >= 576
    static constexpr uint8_t kDhcpT1Value = 58;      ///< T1 renewal time
    static constexpr uint8_t kDhcpT2Value = 59;      ///< T2 renewal time
    static constexpr uint8_t kClientIdentifier = 61;
};

enum class State : uint8_t
{
    kOff = 0,
    kRequesting = 1,
    kInit = 2,
    kRebooting = 3,
    kRebinding = 4,
    kRenewing = 5,
    kSelecting = 6,
    kInforming = 7,
    kChecking = 8,
    kPermanent = 9,
    kBound = 10,
    kReleasing = 11,
    kBackingOff = 12
};

struct Message
{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[network::ip4::kAddressLength];
    uint8_t yiaddr[network::ip4::kAddressLength];
    uint8_t siaddr[network::ip4::kAddressLength];
    uint8_t giaddr[network::ip4::kAddressLength];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[dhcp::kOptSize];
} __attribute__((packed));
} // namespace network::dhcp

#endif /* CORE_PROTOCOL_DHCP_H_ */
