/**
 * @file icmp.h
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

#ifndef CORE_PROTOCOL_ICMP_H_
#define CORE_PROTOCOL_ICMP_H_

#include <cstdint>

#include "core/protocol/ethernet.h"
#include "core/protocol/ip4.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace network::icmp
{
struct Type
{
    static constexpr uint8_t kEchoReply = 0;
    static constexpr uint8_t kEcho = 8;
};

inline constexpr uint8_t kCodeEcho = 0;

inline constexpr uint32_t kHeaderSize = 8;
inline constexpr uint32_t kPayloadSize = (network::ethernet::kMtuSize - kHeaderSize - sizeof(struct network::ip4::Ip4Header));

struct Packet
{
    uint8_t type;         // 1
    uint8_t code;         // 2
    uint16_t checksum;    // 4
    uint8_t parameter[4]; // 8
    uint8_t payload[kPayloadSize];
} PACKED;

struct Header
{
    struct network::ethernet::Header ether;
    struct network::ip4::Ip4Header ip4;
    struct Packet icmp;
} PACKED;

inline constexpr uint32_t kIPv4IcmpHeadersSize = (sizeof(struct Header) - sizeof(struct network::ethernet::Header));
} // namespace network::icmp

#endif /* CORE_PROTOCOL_ICMP_H_ */
