/**
 * @file autoip.cpp
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
/* This code is inspired by the lwIP TCP/IP stack.
 * https://savannah.nongnu.org/projects/lwip/
 */
/**
 * The autoip.cpp aims to be conform to RFC 3927.
 * https://datatracker.ietf.org/doc/html/rfc3927
 * Dynamic Configuration of IPv4 Link-Local Addresses
 */

#ifdef DEBUG_NETWORK_AUTOIP
#undef NDEBUG
#endif

#include <cstring>
#include <cassert>

#include "core/netif.h"
#include "core/ip4/autoip.h"
#include "core/protocol/autoip.h"
#include "core/ip4/acd.h"
#include "firmware/debug/debug_debug.h"

namespace network::autoip
{
static void Bind()
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);
    assert(autoip != nullptr);

    autoip->state = autoip::State::kBound;

    ip4_addr_t sn_mask, gw_addr;

    sn_mask.addr = network::ConvertToUint(255, 255, 0, 0);
    gw_addr.addr = 0;

    netif::SetAddr(autoip->llipaddr, sn_mask, gw_addr);
}

static void Restart()
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);
    assert(autoip != nullptr);

    autoip->tried_llipaddr++;
    autoip::Start();
}

static void ConflictCallback(network::acd::Callback state)
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);
    assert(autoip != nullptr);

    switch (state)
    {
        case network::acd::Callback::kAcdIpOk:
            Bind();
            netif::SetFlags(netif::Netif::kNetifFlagAutoipOk);
            break;
        case network::acd::Callback::kAcdRestartClient:
            Restart();
            break;
        case network::acd::Callback::kAcdDecline:
            // "delete" conflicting address so a new one will be selected in autoip::Start()
            autoip->llipaddr.addr = network::kIpaddrAny;
            autoip::Stop();
            netif::ClearFlags(netif::Netif::kNetifFlagAutoipOk);
            break;
        default:
            break;
    }
}

static void CreateAddr(uint32_t& ipaddr)
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);
    assert(autoip != nullptr);

    // Here we create an IP-Address out of range 169.254.1.0 to 169.254.254.255 compliant to RFC 3927 Section 2.1
    const auto kMask = netif::global::netif_default.hwaddr[3] + (netif::global::netif_default.hwaddr[4] << 8);
    ipaddr = static_cast<uint32_t>(kMask << 16) | autoip::kRangeStart;
    ipaddr = __builtin_bswap32(ipaddr);

    ipaddr += autoip->tried_llipaddr;
    ipaddr = __builtin_bswap32(autoip::kNet) | (ipaddr & 0xffff);

    if (ipaddr < __builtin_bswap32(autoip::kRangeStart))
    {
        ipaddr += __builtin_bswap32(autoip::kRangeEnd) - __builtin_bswap32(autoip::kRangeStart) + 1;
    }

    if (ipaddr > __builtin_bswap32(autoip::kRangeEnd))
    {
        ipaddr -= __builtin_bswap32(autoip::kRangeEnd) - __builtin_bswap32(autoip::kRangeStart) + 1;
    }

    ipaddr = __builtin_bswap32(ipaddr);

    DEBUG_PRINTF(IPSTR, IP2STR(ipaddr));
}

/*
 * Public interface
 */

void Start()
{
    DEBUG_ENTRY();

    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);

    if (autoip == nullptr)
    {
        autoip = new (struct autoip::Autoip);
        assert(autoip != nullptr);
        memset(autoip, 0, sizeof(struct autoip::Autoip));
    }

    if (autoip->state == autoip::State::kOff)
    {
        network::acd::Add(&autoip->acd, ConflictCallback);

        // In accordance to RFC3927 section 2.1:
        // Keep using the same link local address as much as possible.
        // Only when there is none or when there was a conflict, select a new one.
        if (!network::IsLinklocalIp(autoip->llipaddr.addr))
        {
            CreateAddr(autoip->llipaddr.addr);
        }

        autoip->state = autoip::State::kChecking;
        network::acd::Start(&autoip->acd, autoip->llipaddr);
    }
    else
    {
        DEBUG_PUTS("Already started");
    }

    DEBUG_EXIT();
}

void Stop()
{
    DEBUG_ENTRY();
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);

    if (autoip != nullptr)
    {
        autoip->state = autoip::State::kOff;

        ip4_addr_t any;
        any.addr = network::kIpaddrAny;

        if (network::IsLinklocalIp(netif::global::netif_default.ip.addr))
        {
            netif::SetAddr(any, any, any);
        }
    }

    DEBUG_EXIT();
}

bool SuppliedAddress()
{
    const auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);

    return (autoip != nullptr) && (netif::global::netif_default.ip.addr == autoip->llipaddr.addr) && (autoip->state == autoip::State::kBound);
}

void NetworkChangedLinkUp()
{
    DEBUG_ENTRY();

    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);

    if ((autoip != nullptr) && (autoip->state != autoip::State::kOff))
    {
        network::acd::Start(&autoip->acd, autoip->llipaddr);
    }

    DEBUG_EXIT();
}

void NetworkChangedLinkDown()
{
    DEBUG_EXIT();

    const auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::global::netif_default.autoip);

    if ((autoip != nullptr) && (autoip->state != autoip::State::kOff))
    {
        autoip::Stop();
    }

    DEBUG_EXIT();
}
} // namespace network::autoip
