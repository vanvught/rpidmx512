/**
 * @file arp.h
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

#ifndef CORE_PROTOCOL_ARP_H_
#define CORE_PROTOCOL_ARP_H_

#include <cstdint>
#include "core/protocol/ethernet.h"
#include "core/protocol/ieee.h"
#include "core/protocol/ip4.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace network::arp
{
inline constexpr uint16_t kHwtypeEthernet = 1;
inline constexpr uint16_t kPrtypeIPv4 = network::ethernet::Type::kIPv4;
inline constexpr auto kHardwareSize = network::ethernet::kAddressLength;
inline constexpr auto kProtocolSize = network::ip4::kAddressLength;

struct OpCode
{
    static constexpr uint16_t kRqstRqst = 1;
    static constexpr uint16_t kRqstReply = 2;
};

struct ArpPacket
{
    uint16_t hardware_type;                                //  2
    uint16_t protocol_type;                                //  4
    uint8_t hardware_size;                                 //  5
    uint8_t protocol_size;                                 //  6
    uint16_t opcode;                                       //  8
    uint8_t sender_mac[network::ethernet::kAddressLength]; // 14
    uint8_t sender_ip[network::ip4::kAddressLength];       // 18
    uint8_t target_mac[network::ethernet::kAddressLength]; // 24
    uint8_t target_ip[network::ip4::kAddressLength];       // 28
    uint8_t padding[18];                                   // 46  // +14 = 60
} PACKED;

struct Header
{
    struct network::ethernet::Header ether;
    struct ArpPacket arp;
} PACKED;
} // namespace network::arp

#endif // CORE_PROTOCOL_ARP_H_
