/**
 * @file tcp.h
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

#ifndef CORE_PROTOCOL_TCP_H_
#define CORE_PROTOCOL_TCP_H_

#include <cstdint>

#include "core/protocol/ethernet.h"
#include "core/protocol/ip4.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace network::tcp
{

inline constexpr uint16_t kHeaderSize = 20;
inline constexpr uint32_t kDataSize = network::ethernet::kMtuSize - ip4::kHeaderSize - kHeaderSize;

struct Packet
{
    uint16_t srcpt;    //  2
    uint16_t dstpt;    //  4
    uint32_t seqnum;   //  8
    uint32_t acknum;   // 12
    uint8_t offset;    // 13
    uint8_t control;   // 14
    uint16_t window;   // 16
    uint16_t checksum; // 18
    uint16_t urgent;   // 20
    uint8_t data[kDataSize];
} PACKED;

struct Header
{
    struct network::ethernet::Header ether;
    struct network::ip4::Ip4Header ip4;
    struct Packet tcp;
} PACKED;

inline constexpr uint16_t kTcpOptTs = 12;                                                                         // NOP,NOP,TS(10)
inline constexpr uint16_t kTcpOptSyn = 16;                                                                        // MSS(4) + TS(12)
inline constexpr uint16_t kTcpDataMss = network::ethernet::kMtuSize - ip4::kHeaderSize - kHeaderSize - kTcpOptTs; // 1448
inline constexpr uint16_t kTcpSynMss = network::ethernet::kMtuSize - ip4::kHeaderSize - kHeaderSize - kTcpOptSyn; // 1444
} // namespace network::tcp

#endif // CORE_PROTOCOL_TCP_H_
