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

#ifdef DEBUG_AUTOIP
#undef NDEBUG
#endif

#include <cstring>
#include <cassert>

#include "net/netif.h"
#include "net/autoip.h"
#include "net/protocol/autoip.h"
#include "net/acd.h"
#include "firmware/debug/debug_debug.h"

namespace net
{
static void Bind()
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);
    assert(autoip != nullptr);

    autoip->state = autoip::State::AUTOIP_STATE_BOUND;

    ip4_addr_t sn_mask, gw_addr;

    sn_mask.addr = net::convert_to_uint(255, 255, 0, 0);
    gw_addr.addr = 0;

    netif::SetAddr(autoip->llipaddr, sn_mask, gw_addr);
}

static void Restart()
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);
    assert(autoip != nullptr);

    autoip->tried_llipaddr++;
    autoip::Start();
}

static void ConflictCallback(net::acd::Callback state)
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);
    assert(autoip != nullptr);

    switch (state)
    {
        case net::acd::Callback::ACD_IP_OK:
            Bind();
            netif::SetFlags(netif::Netif::kNetifFlagAutoipOk);
            break;
        case net::acd::Callback::ACD_RESTART_CLIENT:
            Restart();
            break;
        case net::acd::Callback::ACD_DECLINE:
            // "delete" conflicting address so a new one will be selected in autoip::Start()
            autoip->llipaddr.addr = net::IPADDR_ANY;
            autoip::Stop();
            netif::ClearFlags(netif::Netif::kNetifFlagAutoipOk);
            break;
        default:
            break;
    }
}

static void CreateAddr(uint32_t& ipaddr)
{
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);
    assert(autoip != nullptr);

    /* Here we create an IP-Address out of range 169.254.1.0 to 169.254.254.255
     * compliant to RFC 3927 Section 2.1 */
    const auto kMask = netif::globals::netif_default.hwaddr[3] + (netif::globals::netif_default.hwaddr[4] << 8);
    ipaddr = static_cast<uint32_t>(kMask << 16) | autoip::AUTOIP_RANGE_START;
    ipaddr = __builtin_bswap32(ipaddr);

    ipaddr += autoip->tried_llipaddr;
    ipaddr = __builtin_bswap32(autoip::AUTOIP_NET) | (ipaddr & 0xffff);

    if (ipaddr < __builtin_bswap32(autoip::AUTOIP_RANGE_START))
    {
        ipaddr += __builtin_bswap32(autoip::AUTOIP_RANGE_END) - __builtin_bswap32(autoip::AUTOIP_RANGE_START) + 1;
    }

    if (ipaddr > __builtin_bswap32(autoip::AUTOIP_RANGE_END))
    {
        ipaddr -= __builtin_bswap32(autoip::AUTOIP_RANGE_END) - __builtin_bswap32(autoip::AUTOIP_RANGE_START) + 1;
    }

    ipaddr = __builtin_bswap32(ipaddr);

    DEBUG_PRINTF(IPSTR, IP2STR(ipaddr));
}

/*
 * Public interface
 */

void autoip::Start()
{
    DEBUG_ENTRY();

    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);

    if (autoip == nullptr)
    {
        autoip = new (struct autoip::Autoip);
        assert(autoip != nullptr);
        memset(autoip, 0, sizeof(struct autoip::Autoip));
    }

    if (autoip->state == autoip::State::AUTOIP_STATE_OFF)
    {
        net::acd::Add(&autoip->acd, ConflictCallback);

        /* In accordance to RFC3927 section 2.1:
         * Keep using the same link local address as much as possible.
         * Only when there is none or when there was a conflict, select a new one.
         */
        if (!net::is_linklocal_ip(autoip->llipaddr.addr))
        {
            CreateAddr(autoip->llipaddr.addr);
        }

        autoip->state = autoip::State::AUTOIP_STATE_CHECKING;
        net::acd::Start(&autoip->acd, autoip->llipaddr);
    }
    else
    {
        DEBUG_PUTS("Already started");
    }

    DEBUG_EXIT();
}

void autoip::Stop()
{
    DEBUG_ENTRY();
    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);

    if (autoip != nullptr)
    {
        autoip->state = autoip::State::AUTOIP_STATE_OFF;

        ip4_addr_t any;
        any.addr = net::IPADDR_ANY;

        if (net::is_linklocal_ip(netif::globals::netif_default.ip.addr))
        {
            netif::SetAddr(any, any, any);
        }
    }

    DEBUG_EXIT();
}
} // namespace net
