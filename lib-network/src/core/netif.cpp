/**
 * @file netif.cpp
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

#if defined(DEBUG_NET_NETIF)
#undef NDEBUG
#endif

#include "net_private.h"
#include "core/netif.h"
#include "core/ip4/acd.h"
#include "core/ip4/autoip.h"
#include "core/ip4/dhcp.h"
#include "firmware/debug/debug_debug.h"

namespace network::igmp
{
void ReportGroups();
}
namespace netif
{
namespace global
{
struct Netif netif_default;
} // namespace global

static netif_ext_callback_fn callback_fn;

static void DefaultCallback([[maybe_unused]] uint16_t reason, [[maybe_unused]] const netif_ext_callback_args_t* args)
{
    DEBUG_PRINTF("%u", reason);
}

void Init()
{
    auto& netif = netif::global::netif_default;

    netif.ip.addr = 0;
    netif.netmask.addr = 0;
    netif.gw.addr = 0;
    netif.broadcast_ip.addr = 0;
    netif.secondary_ip.addr = 2 + ((static_cast<uint32_t>(static_cast<uint8_t>(netif.hwaddr[3] + 0xFF + 0xFF))) << 8) + ((static_cast<uint32_t>(netif.hwaddr[4])) << 16) + ((static_cast<uint32_t>(netif.hwaddr[5])) << 24);
    netif.flags = 0;
    netif.dhcp = nullptr;
    netif.acd = nullptr;
    netif.autoip = nullptr;

    callback_fn = &DefaultCallback;
}

static void NetifDoUpdateGlobals()
{
    auto& netif = netif::global::netif_default;
    netif.broadcast_ip.addr = (netif.ip.addr | ~netif.netmask.addr);

    network::global::broadcast_mask = ~(netif.netmask.addr);
    network::global::on_network_mask = netif.ip.addr & netif.netmask.addr;
}

static void NetifDoIpAddrChanged([[maybe_unused]] network::ip4_addr_t old_addr, [[maybe_unused]] network::ip4_addr_t new_addr)
{
    //  tcp_netif_ip_addr_changed(old_addr, new_addr);
    //  udp_netif_ip_addr_changed(old_addr, new_addr);
}

static void NetifIssueReports()
{
    const auto& netif = netif::global::netif_default;

    if (!(netif.flags & Netif::kNetifFlagLinkUp))
    {
        return;
    }

    if (netif.ip.addr != 0)
    {
        network::igmp::ReportGroups();
    }
}

static bool NetifDoSetIpaddr(network::ip4_addr_t ipaddr, network::ip4_addr_t& old_addr)
{
    DEBUG_ENTRY();

    auto& netif = netif::global::netif_default;

    DEBUG_PRINTF(IPSTR " " IPSTR, IP2STR(ipaddr.addr), IP2STR(netif.ip.addr));

    // Update the address if it's different
    if (ipaddr.addr != netif.ip.addr)
    {
        old_addr.addr = netif.ip.addr;

        NetifDoIpAddrChanged(old_addr, ipaddr);
        network::acd::NetifIpAddrChanged(old_addr, ipaddr);

        netif.ip.addr = ipaddr.addr;

        NetifDoUpdateGlobals();
        NetifIssueReports();

        DEBUG_EXIT();
        return true; // address changed
    }

    DEBUG_EXIT();
    return false; // address unchanged
}

static bool NetifDoSetNetmask(network::ip4_addr_t netmask, network::ip4_addr_t& old_nm)
{
    DEBUG_ENTRY();
    auto& netif = netif::global::netif_default;

    if (netmask.addr != netif.netmask.addr)
    {
        old_nm.addr = netif.netmask.addr;
        netif.netmask.addr = netmask.addr;

        NetifDoUpdateGlobals();

        DEBUG_EXIT();
        return true; // netmask changed
    }

    DEBUG_EXIT();
    return false; // netmask unchanged
}

static bool NetifDoSetGw(network::ip4_addr_t gw, network::ip4_addr_t& old_gw)
{
    DEBUG_ENTRY();

    auto& netif = netif::global::netif_default;

    if (gw.addr != netif.gw.addr)
    {
        old_gw.addr = netif.gw.addr;
        netif.gw.addr = gw.addr;

        DEBUG_EXIT();
        return true; // gateway changed
    }

    DEBUG_EXIT();
    return false; // gateway unchanged
}

void SetIpAddr(network::ip4_addr_t ipaddr)
{
    network::ip4_addr_t old_addr;

    if (NetifDoSetIpaddr(ipaddr, old_addr))
    {
        netif_ext_callback_args_t args;
        args.ipv4_changed.old_address.addr = old_addr.addr;
        callback_fn(NetifReason::kIpv4AddressChanged, &args);
    }
}

void SetNetmask(network::ip4_addr_t netmask)
{
    network::ip4_addr_t old_nm;

    if (NetifDoSetNetmask(netmask, old_nm))
    {
        netif_ext_callback_args_t args;
        args.ipv4_changed.old_netmask = old_nm;
        callback_fn(NetifReason::kIpv4NetmaskChanged, &args);
    }
}

void SetGw(network::ip4_addr_t gw)
{
    network::ip4_addr_t old_gw;

    if (NetifDoSetGw(gw, old_gw))
    {
        netif_ext_callback_args_t args;
        args.ipv4_changed.old_gw = old_gw;
        callback_fn(NetifReason::kIpv4GatewayChanged, &args);
    }
}

void SetAddr(network::ip4_addr_t ipaddr, network::ip4_addr_t netmask, network::ip4_addr_t gw)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR " " IPSTR " " IPSTR, IP2STR(ipaddr.addr), IP2STR(netmask.addr), IP2STR(gw.addr));

    auto change_reason = NetifReason::kNone;
    netif_ext_callback_args_t cb_args;

    network::ip4_addr_t old_addr;
    network::ip4_addr_t old_nm;
    network::ip4_addr_t old_gw;

    const auto kRemove = (ipaddr.addr == 0);

    if (kRemove)
    {
        /* when removing an address, we have to remove it *before* changing netmask/gw
         to ensure that tcp RST segment can be sent correctly */
        if (NetifDoSetIpaddr(ipaddr, old_addr))
        {
            change_reason |= NetifReason::kIpv4AddressChanged;
            cb_args.ipv4_changed.old_address.addr = old_addr.addr;
        }
    }

    if (NetifDoSetNetmask(netmask, old_nm))
    {
        change_reason |= NetifReason::kIpv4NetmaskChanged;
        cb_args.ipv4_changed.old_netmask.addr = old_nm.addr;
    }

    if (NetifDoSetGw(gw, old_gw))
    {
        change_reason |= NetifReason::kIpv4GatewayChanged;
        cb_args.ipv4_changed.old_gw = old_gw;
    }

    if (!kRemove)
    {
        /* set ipaddr last to ensure netmask/gw have been set when status callback is called */
        if (NetifDoSetIpaddr(ipaddr, old_addr))
        {
            change_reason |= NetifReason::kIpv4AddressChanged;
            cb_args.ipv4_changed.old_address.addr = old_addr.addr;
        }
    }

    if (change_reason != NetifReason::kNone)
    {
        change_reason |= NetifReason::kIpv4SettingsChanged;
    }

    if (!kRemove)
    {
        /* Issue a callback even if the address hasn't changed, eg. DHCP reboot */
        change_reason |= NetifReason::kIpv4AddressValid;
    }

    DEBUG_PRINTF("change_reason=%u", change_reason);

    if (change_reason != NetifReason::kNone)
    {
        callback_fn(change_reason, &cb_args);
    }

    DEBUG_EXIT();
}

void AddExtCallback(netif_ext_callback_fn fn)
{
    callback_fn = fn;
}

// Link

void SetLinkUp()
{
    DEBUG_ENTRY();
    const auto& netif = netif::global::netif_default;

    if (!(netif.flags & Netif::kNetifFlagLinkUp))
    {
        netif::SetFlags(Netif::kNetifFlagLinkUp);

        network::dhcp::NetworkChangedLinkUp();
        network::autoip::NetworkChangedLinkUp();

        NetifIssueReports();

        netif_ext_callback_args_t args;
        args.link_changed.state = 1;
        callback_fn(NetifReason::kLinkChanged, &args);

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void SetLinkDown()
{
    DEBUG_ENTRY();

    const auto& netif = netif::global::netif_default;

    if (netif.flags & Netif::kNetifFlagLinkUp)
    {
        netif::ClearFlags(Netif::kNetifFlagLinkUp);

        network::autoip::NetworkChangedLinkDown();

        network::acd::NetworkChangedLinkDown();

        netif_ext_callback_args_t args;
        args.link_changed.state = 0;
        callback_fn(NetifReason::kLinkChanged, &args);

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}
} // namespace netif
