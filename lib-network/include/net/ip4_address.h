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

#ifndef NET_IP4_ADDRESS_H_
#define NET_IP4_ADDRESS_H_

#include <cstdint>

namespace net
{
struct ip_addr
{
    uint32_t addr;
};

typedef struct ip_addr ip4_addr_t;

#define IP2STR(addr) \
    static_cast<int>(addr & 0xFF), static_cast<int>((addr >> 8) & 0xFF), static_cast<int>((addr >> 16) & 0xFF), static_cast<int>((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) \
    static_cast<int>(mac[0]), static_cast<int>(mac[1]), static_cast<int>(mac[2]), static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

static constexpr uint32_t MAC_SIZE = 6;
static constexpr uint32_t HOSTNAME_SIZE = 64;   ///< Including a terminating null byte.
static constexpr uint32_t DOMAINNAME_SIZE = 64; ///< Including a terminating null byte.
static constexpr uint32_t NAMESERVERS_COUNT = 3;

static constexpr uint32_t convert_to_uint(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d)
{
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(c) << 16 | static_cast<uint32_t>(d) << 24;
}

static constexpr uint32_t IPADDR_NONE = convert_to_uint(255, 255, 255, 255);
static constexpr uint32_t IPADDR_LOOPBACK = convert_to_uint(127, 0, 0, 1);
static constexpr uint32_t IPADDR_ANY = convert_to_uint(0, 0, 0, 0);
static constexpr uint32_t IPADDR_BROADCAST = convert_to_uint(255, 255, 255, 255);

inline bool is_netmask_valid(uint32_t nNetMask)
{
    if (nNetMask == 0)
    {
        return false;
    }
    nNetMask = __builtin_bswap32(nNetMask);
    return !(nNetMask & (~nNetMask >> 1));
}
/**
 * The private address ranges are defined in RFC1918.
 */
inline bool is_private_ip(const uint32_t nIp)
{
    const uint8_t n = (nIp >> 8) & 0xFF;

    switch (nIp & 0xFF)
    {
        case 10:
            return true;
            break;
        case 172:
            return (n >= 16) && (n < 32);
        case 192:
            return n == 168;
        default:
            break;
    }

    return false;
}

inline constexpr bool is_linklocal_ip(uint32_t ip)
{
    return (ip & 0xFFFF) == 0xA9FE;
}

inline constexpr bool is_multicast_ip(uint32_t ip)
{
    return (ip & 0xF0) == 0xE0;
}

inline uint32_t cidr_to_netmask(uint8_t cidr)
{
    if (cidr != 0)
    {
        const auto nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - cidr));
        return nNetmask;
    }

    return 0;
}
} // namespace net

#endif  // NET_IP4_ADDRESS_H_
