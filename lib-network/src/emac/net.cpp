/**
 * @file net.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET)
#undef NDEBUG
#endif

#include <cstdint>

#include "network_net.h"
#include "emac/phy.h"
#include "net/netif.h"
#include "net/dhcp.h"
#include "net/apps/mdns.h"
#include "network_store.h"
#include "firmware/debug/debug_debug.h"

#if !defined(PHY_ADDRESS)
#define PHY_ADDRESS 1
#endif

namespace net
{
namespace globals
{
uint32_t broadcast_mask;
uint32_t on_network_mask;
} // namespace globals

static struct net::acd::Acd s_acd;

static void PrimaryIpConflictCallback(net::acd::Callback callback)
{
    auto& netif = netif::globals::netif_default;

    switch (callback)
    {
        case net::acd::Callback::ACD_IP_OK:
            if (s_acd.ipaddr.addr == netif.secondary_ip.addr)
            {
                net::SetSecondaryIp();
            }
            else
            {
                netif::SetIpAddr(s_acd.ipaddr);
            }
            net::dhcp::Inform();
            netif::SetFlags(netif::Netif::kNetifFlagStaticipOk);
            break;
        case net::acd::Callback::ACD_RESTART_CLIENT:
            break;
        case net::acd::Callback::ACD_DECLINE:
            netif::ClearFlags(netif::Netif::kNetifFlagStaticipOk);
            break;
        default:
            break;
    }
}

void Set(net::ip4_addr_t ipaddr, net::ip4_addr_t netmask, net::ip4_addr_t gw, bool use_dhcp)
{
    DEBUG_ENTRY();

    netif::globals::netif_default.secondary_ip.addr =
        2 + ((static_cast<uint32_t>(static_cast<uint8_t>(netif::globals::netif_default.hwaddr[3] + 0xFF + 0xFF))) << 8) +
        ((static_cast<uint32_t>(netif::globals::netif_default.hwaddr[4])) << 16) + ((static_cast<uint32_t>(netif::globals::netif_default.hwaddr[5])) << 24);

    if (!use_dhcp)
    {
        net::acd::Add(&s_acd, PrimaryIpConflictCallback);

        if (ipaddr.addr != 0)
        {
            netif::SetNetmask(netmask);
            netif::SetGw(gw);
        }
    }

    if (net::phy::Link::kStateUp == net::phy::GetLink(PHY_ADDRESS))
    {
        netif::SetFlags(netif::Netif::kNetifFlagLinkUp);
    }
    else
    {
        netif::ClearFlags(netif::Netif::kNetifFlagLinkUp);
    }

    if (use_dhcp)
    {
        net::dhcp::Start();
    }
    else
    {
        if (ipaddr.addr == 0)
        {
            net::acd::Start(&s_acd, netif::globals::netif_default.secondary_ip);
        }
        else
        {
            net::acd::Start(&s_acd, ipaddr);
        }
    }

    DEBUG_EXIT();
}

void SetPrimaryIp(uint32_t primary_ip_new)
{
    DEBUG_ENTRY();

    auto& netif = netif::globals::netif_default;

    if (primary_ip_new == netif.ip.addr)
    {
        DEBUG_EXIT();
        return;
    }

    net::dhcp::ReleaseAndStop();

    network::store::SaveDhcp(false);

    net::acd::Add(&s_acd, PrimaryIpConflictCallback);

    if (primary_ip_new == 0)
    {
        net::acd::Start(&s_acd, netif.secondary_ip);
    }
    else
    {
        net::ip_addr ipaddr;
        ipaddr.addr = primary_ip_new;
        net::acd::Start(&s_acd, ipaddr);
    }

    network::store::SaveIp(primary_ip_new);

    DEBUG_EXIT();
}

void SetSecondaryIp()
{
    DEBUG_ENTRY();

    auto& netif = netif::globals::netif_default;
    net::ip4_addr_t netmask;
    netmask.addr = 255;
    netif::SetAddr(netif.secondary_ip, netmask, netif.secondary_ip);

    DEBUG_EXIT();
}

void SetNetmask(uint32_t netmask_new)
{
    DEBUG_ENTRY();

    if (netmask_new == netif::Netmask())
    {
        DEBUG_EXIT();
        return;
    }

    net::ip4_addr_t netmask;
    netmask.addr = netmask_new;

    netif::SetNetmask(netmask);

    network::store::SaveNetmask(netmask_new);

    DEBUG_EXIT();
}

void SetGatewayIp(uint32_t gw_new)
{
    DEBUG_ENTRY();

    if (gw_new == netif::Gw())
    {
        DEBUG_EXIT();
        return;
    }

    net::ip4_addr_t gw;
    gw.addr = gw_new;

    netif::SetGw(gw);

    network::store::SaveGatewayIp(gw_new);

    DEBUG_EXIT();
}

namespace igmp
{
void Shutdown();
} // namespace igmp

void Shutdown()
{
    DEBUG_ENTRY();

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    mdns::Stop();
#endif
    net::igmp::Shutdown();
    netif::SetLinkDown();

    DEBUG_EXIT();
}

} // namespace net
