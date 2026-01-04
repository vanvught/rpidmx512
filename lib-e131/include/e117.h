/**
 * @file e117.h
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef E117_H_
#define E117_H_

#include <cstdint>

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace e117
{
inline constexpr uint32_t kAcnPacketIdentifierLength = 12;
inline constexpr uint8_t kAcnPacketIdentifier[kAcnPacketIdentifierLength] = ///< 5.3 ACN Packet Identifier
    {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};

inline constexpr uint32_t kCidLength = 16;
/**
 * Root Layer (See Section 5)
 */
struct RootLayer
{
    uint16_t pre_amble_size;                                   ///< Define RLP Preamble Size. Fixed 0x0010
    uint16_t post_amble_size;                                  ///< RLP Post-amble Size. Fixed 0x0000
    uint8_t acn_packet_identifier[kAcnPacketIdentifierLength]; ///< ACN Packet Identifier
    uint16_t flags_length;                                     ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;                                           ///< Identifies RLP Data as 1.31 Protocol PDU 0x00000004
    uint8_t cid[kCidLength];                                   ///< Sender's CID. Sender's unique ID
} PACKED;

inline constexpr auto kRootLayerSize = sizeof(struct RootLayer);
} // namespace e117

#endif  // E117_H_
