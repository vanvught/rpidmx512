/**
 * @file netif.h
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

#ifndef CORE_NETIF_H_
#define CORE_NETIF_H_

#include <cstdint>

#include "ip4/ip4_address.h"

#ifndef NETIF_MAX_HWADDR_LEN
#define NETIF_MAX_HWADDR_LEN 6U
#endif

namespace netif
{

struct Netif
{
    static constexpr uint8_t kNetifFlagLinkUp = (1U << 0);
    static constexpr uint8_t kNetifFlagDhcpOk = (1U << 1);
    static constexpr uint8_t kNetifFlagAutoipOk = (1U << 2);
    static constexpr uint8_t kNetifFlagStaticipOk = (1U << 3);

    struct network::ip_addr ip;
    struct network::ip_addr netmask;
    struct network::ip_addr gw;
    struct network::ip_addr broadcast_ip;
    struct network::ip_addr secondary_ip;

    uint8_t hwaddr[NETIF_MAX_HWADDR_LEN];
    uint8_t flags;

    const char* hostname;

    void* dhcp;
    void* acd;
    void* autoip;
};

namespace global
{
extern struct Netif netif_default;
} // namespace global

struct NetifReason
{
    static constexpr uint16_t kNone = 0x0000;
    static constexpr uint16_t kLinkChanged = 0x0004;
    static constexpr uint16_t kIpv4AddressChanged = 0x0010;
    static constexpr uint16_t kIpv4GatewayChanged = 0x0020;
    static constexpr uint16_t kIpv4NetmaskChanged = 0x0040;
    static constexpr uint16_t kIpv4SettingsChanged = 0x0080;
    static constexpr uint16_t kIpv4AddressValid = 0x0400;
};

struct Ipv4Changed
{
    network::ip4_addr_t old_address;
    network::ip4_addr_t old_netmask;
    network::ip4_addr_t old_gw;
};

struct LinkChanged
{
    /** 1: up; 0: down */
    uint8_t state;
};

union netif_ext_callback_args_t
{
    struct LinkChanged link_changed;
    struct Ipv4Changed ipv4_changed;
};

typedef void (*netif_ext_callback_fn)(uint16_t reason, const netif_ext_callback_args_t* args);

inline void SetFlags(uint8_t flags)
{
    global::netif_default.flags |= flags;
}

inline void ClearFlags(uint8_t flags)
{
    global::netif_default.flags &= static_cast<uint8_t>(~flags);
}

inline uint32_t IpAddr()
{
    return netif::global::netif_default.ip.addr;
}

inline uint32_t SecondaryIpAddr()
{
    return netif::global::netif_default.secondary_ip.addr;
}

inline const uint8_t* HwAddr()
{
    return netif::global::netif_default.hwaddr;
}

void Init();
void SetIpAddr(network::ip4_addr_t ipaddr);
void SetNetmask(network::ip4_addr_t netmask);

inline uint32_t Netmask()
{
    return netif::global::netif_default.netmask.addr;
}

void SetGw(network::ip4_addr_t gw);

inline uint32_t Gw()
{
    return netif::global::netif_default.gw.addr;
}

void SetAddr(network::ip4_addr_t ipaddr, network::ip4_addr_t netmask, network::ip4_addr_t gw);

void AddExtCallback(netif_ext_callback_fn fn);

inline uint32_t BroadcastIpAddr()
{
    return netif::global::netif_default.broadcast_ip.addr;
}

// Link
void SetLinkUp();
void SetLinkDown();

inline bool IsLinkUp()
{
    return (global::netif_default.flags & Netif::kNetifFlagLinkUp) == Netif::kNetifFlagLinkUp;
}
} // namespace netif

#endif // CORE_NETIF_H_
