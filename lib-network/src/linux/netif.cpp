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

#include "net/netif.h"
#include "firmware/debug/debug_debug.h"

namespace net::globals
{
uint32_t broadcast_mask;
uint32_t on_network_mask;
} // namespace net::globals

namespace netif::globals
{
	struct Netif netif_default;
}

namespace netif
{
static void NetifDoUpdateGlobals()
{
    auto& netif = netif::globals::netif_default;
    netif.broadcast_ip.addr = (netif.ip.addr | ~netif.netmask.addr);

    net::globals::broadcast_mask = ~(netif.netmask.addr);
    net::globals::on_network_mask = netif.ip.addr & netif.netmask.addr;
}

static bool NetifDoSetNetmask(net::ip4_addr_t netmask, net::ip4_addr_t& old_nm)
{
    DEBUG_ENTRY();
    auto& netif = netif::globals::netif_default;

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

static bool NetifDoSetGw(net::ip4_addr_t gw, net::ip4_addr_t& old_gw)
{
    DEBUG_ENTRY();
    auto& netif = netif::globals::netif_default;

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

void SetNetmask(net::ip4_addr_t netmask)
{
    net::ip4_addr_t old_nm;

    if (NetifDoSetNetmask(netmask, old_nm))
    {
        netif::netif_ext_callback_args_t args;
        args.ipv4_changed.old_netmask = old_nm;
    }
}

void SetGw(net::ip4_addr_t gw)
{
    net::ip4_addr_t old_gw;

    if (NetifDoSetGw(gw, old_gw))
    {
        netif::netif_ext_callback_args_t args;
        args.ipv4_changed.old_gw = old_gw;
    }
}
void Init()
{
    auto& netif = netif::globals::netif_default;

    netif.ip.addr = 0;
    netif.netmask.addr = 0;
    netif.gw.addr = 0;
    netif.broadcast_ip.addr = 0;
    netif.secondary_ip.addr = 2 + ((static_cast<uint32_t>(static_cast<uint8_t>(netif.hwaddr[3] + 0xFF + 0xFF))) << 8) +
                              ((static_cast<uint32_t>(netif.hwaddr[4])) << 16) + ((static_cast<uint32_t>(netif.hwaddr[5])) << 24);
    netif.flags = 0;
    netif.dhcp = nullptr;
    netif.acd = nullptr;
    netif.autoip = nullptr;
}
} // namespace netif
