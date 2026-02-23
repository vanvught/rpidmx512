/**
 * @file ip4.h
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
#ifndef CORE_PROTOCOL_IP4_H_
#define CORE_PROTOCOL_IP4_H_

#include <cstdint>

#include "core/protocol/ethernet.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace network::ip4
{
inline constexpr uint32_t kAddressLength = 4;

struct Flags
{
    static constexpr uint16_t kFlagLf = 0x0000;
    static constexpr uint16_t kFlagMf = 0x2000;
    static constexpr uint16_t kFlagDf = 0x4000;
};

struct Proto
{
    static constexpr uint8_t kIcmp = 1;
    static constexpr uint8_t kIgmp = 2;
    static constexpr uint8_t kTcp = 6;
    static constexpr uint8_t kUdp = 17;
};

struct Ip4Header
{
    uint8_t ver_ihl;             //  1
    uint8_t tos;                 //  2
    uint16_t len;                //  4
    uint16_t id;                 //  6
    uint16_t flags_froff;        //  8
    uint8_t ttl;                 //  9
    uint8_t proto;               // 10
    uint16_t chksum;             // 12
    uint8_t src[kAddressLength]; // 16
    uint8_t dst[kAddressLength]; // 20
} PACKED;

inline constexpr uint32_t kHeaderSize = sizeof(struct Ip4Header);

struct Header
{
    struct ethernet::Header ether;
    struct Ip4Header ip4;
} PACKED;
} // namespace network::ip4

#endif // CORE_PROTOCOL_IP4_H_
