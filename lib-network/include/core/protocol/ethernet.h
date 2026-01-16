/**
 * @file ethernet.h
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

#ifndef CORE_PROTOCOL_ETHERNET_H_
#define CORE_PROTOCOL_ETHERNET_H_

#include <stdint.h>

namespace network::ethernet
{
inline constexpr uint32_t kMtuSize = 1500;
inline constexpr uint32_t kAddressLength = 6;
// The 24-bit IANA IPv4-multicast OUI is 01-00-5e:
inline constexpr uint8_t kIP4MulticastAddr0 = 0x01;
inline constexpr uint8_t kIP4MulticastAddr1 = 0x00;
inline constexpr uint8_t kIP4MulticastAddr2 = 0x5e;

struct Header
{
    uint8_t dst[kAddressLength]; //  6
    uint8_t src[kAddressLength]; // 12
    uint16_t type;               // 14
} __attribute__((packed));
} // namespace network::ethernet

#endif // CORE_PROTOCOL_ETHERNET_H_
