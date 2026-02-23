/**
 * @file udp.h
 *
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

#ifndef CORE_PROTOCOL_UDP_H_
#define CORE_PROTOCOL_UDP_H_

#include <cstdint>

#include "core/protocol/ethernet.h"
#include "core/protocol/ip4.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace network::udp
{

inline constexpr uint32_t kHeaderSize = 8;
inline constexpr uint32_t kDataSize = network::ethernet::kMtuSize - ip4::kHeaderSize - kHeaderSize;

struct Packet
{
    uint16_t source_port;      // 2
    uint16_t destination_port; // 4
    uint16_t len;              // 6
    uint16_t checksum;         // 8
    uint8_t data[kDataSize];
} PACKED;

struct Header
{
    struct network::ethernet::Header ether;
    struct network::ip4::Ip4Header ip4;
    struct Packet udp;
} PACKED;

inline constexpr uint32_t kIPv4UdpHeadersSize = (sizeof(struct network::ip4::Ip4Header) + kHeaderSize);             // IP | UDP
inline constexpr uint32_t kUdpPacketHeadersSize = (sizeof(struct network::ethernet::Header) + kIPv4UdpHeadersSize); // ETH | IP | UDP
} // namespace network::udp

#endif // CORE_PROTOCOL_UDP_H_
