/**
 * @file ip4_address.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef IP4_IP4_ADDRESS_H_
#define IP4_IP4_ADDRESS_H_

#include <cstdint>

namespace network
{
struct ip_addr
{
    uint32_t addr;
};

typedef struct ip_addr ip4_addr_t;

#define IP2STR(addr) static_cast<int>(addr & 0xFF), static_cast<int>((addr >> 8) & 0xFF), static_cast<int>((addr >> 16) & 0xFF), static_cast<int>((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) static_cast<int>(mac[0]), static_cast<int>(mac[1]), static_cast<int>(mac[2]), static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

inline constexpr uint32_t ConvertToUint(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(c) << 16 | static_cast<uint32_t>(d) << 24;
}

inline constexpr uint32_t kIpaddrNone = ConvertToUint(255, 255, 255, 255);
inline constexpr uint32_t kIpaddrLoopback = ConvertToUint(127, 0, 0, 1);
inline constexpr uint32_t kIpaddrAny = ConvertToUint(0, 0, 0, 0);
inline constexpr uint32_t kIpaddrBroadcast = ConvertToUint(255, 255, 255, 255);

inline bool IsNetmaskValid(uint32_t netmask)
{
    if (netmask == 0)
    {
        return false;
    }
    netmask = __builtin_bswap32(netmask);
    return !(netmask & (~netmask >> 1));
}

// The private address ranges are defined in RFC1918.
inline bool IsPrivateIp(uint32_t ip)
{
    const uint8_t kN = (ip >> 8) & 0xFF;

    switch (ip & 0xFF)
    {
        case 10:
            return true;
            break;
        case 172:
            return (kN >= 16) && (kN < 32);
        case 192:
            return kN == 168;
        default:
            break;
    }

    return false;
}

inline constexpr bool IsLinklocalIp(uint32_t ip)
{
    return (ip & 0xFFFF) == 0xA9FE;
}

inline constexpr bool IsMulticastIp(uint32_t ip)
{
    return (ip & 0xF0) == 0xE0;
}

inline uint32_t CidrToNetmask(uint8_t cidr)
{
    if (cidr != 0)
    {
        const auto kNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - cidr));
        return kNetmask;
    }

    return 0;
}
} // namespace network

#endif // IP4_IP4_ADDRESS_H_
