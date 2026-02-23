/**
 * @file network_config.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NETWORK_CONFIG_H_
#define NETWORK_CONFIG_H_

#include <cstdint>

#include "core/netif.h"

namespace network
{
void SetPrimaryIp(uint32_t ip);
void SetSecondaryIp();
void SetNetmask(uint32_t netmask);
void SetGatewayIp(uint32_t gateway_ip);

inline uint32_t GetPrimaryIp()
{
    return netif::IpAddr();
}

inline uint32_t GetSecondaryIp()
{
    return netif::SecondaryIpAddr();
}

inline uint32_t GetNetmask()
{
    return netif::Netmask();
}

inline uint32_t GetGatewayIp()
{
    return netif::Gw();
}

inline uint32_t GetBroadcastIp()
{
    return netif::BroadcastIpAddr();
}

inline uint32_t GetNetmaskCIDR()
{
    return static_cast<uint32_t>(__builtin_popcount(GetNetmask()));
}

inline bool IsValidIp(uint32_t ip)
{
    return (netif::IpAddr() & netif::Netmask()) == (ip & netif::Netmask());
}

void SetAutoIp();

void Shutdown();

} // namespace network

#endif // NETWORK_CONFIG_H_
