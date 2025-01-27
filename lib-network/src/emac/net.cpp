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

#if defined (DEBUG_NET)
# if defined (NDEBUG)
#  undef NDEBUG
# endif
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>

#include "net.h"
#include "net/acd.h"
#include "net/autoip.h"
#include "net/dhcp.h"

#include "network_store.h"

#include "debug.h"

static struct net::acd::Acd s_acd;

namespace net {
static void primary_ip_conflict_callback(net::acd::Callback callback) {
	auto &netif = net::globals::netif_default;

	switch (callback) {
	case net::acd::Callback::ACD_IP_OK:
		if (s_acd.ipaddr.addr == netif.secondary_ip.addr) {
			net_set_secondary_ip();
		} else {
			net::netif_set_ipaddr(s_acd.ipaddr);
		}
		dhcp_inform();
		netif_set_flags(netif::NETIF_FLAG_STATICIP_OK);
		break;
	case net::acd::Callback::ACD_RESTART_CLIENT:
		break;
	case net::acd::Callback::ACD_DECLINE:
		netif_clear_flags(netif::NETIF_FLAG_STATICIP_OK);
		break;
	default:
		break;
	}
}

void net_set_primary_ip(const uint32_t nIp) {
	DEBUG_ENTRY

	auto &netif = net::globals::netif_default;

	if (nIp == netif.ip.addr) {
		DEBUG_EXIT
		return;
	}

	net::dhcp_release_and_stop();
	network_store_save_dhcp(false);

	acd_add(&s_acd, primary_ip_conflict_callback);

	if (nIp  == 0) {
		acd_start(&s_acd, netif.secondary_ip);
	} else {
		ip_addr ipaddr;
		ipaddr.addr = nIp;
		acd_start(&s_acd, ipaddr);
	}

	network_store_save_ip(nIp);

	DEBUG_EXIT
}

void net_set_secondary_ip() {
	DEBUG_ENTRY

	auto &netif = net::globals::netif_default;
	ip4_addr_t netmask;
	netmask.addr = 255;
	net::netif_set_addr(netif.secondary_ip, netmask, netif.secondary_ip);

	DEBUG_EXIT
}

void net_set_netmask(const uint32_t nNetmask) {
	DEBUG_ENTRY

	if (nNetmask == net::netif_netmask()) {
		DEBUG_EXIT
		return;
	}

	net::ip4_addr_t netmask;
	netmask.addr = nNetmask;

	net::netif_set_netmask(netmask);

	network_store_save_netmask(nNetmask);

	DEBUG_EXIT
}

void net_set_gateway_ip(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (nGatewayIp == net::netif_gw()) {
		DEBUG_EXIT
		return;
	}

	net::ip4_addr_t gw;
	gw.addr = nGatewayIp;

	net::netif_set_gw(gw);

	network_store_save_gatewayip(nGatewayIp);

	DEBUG_EXIT
}

void net_enable_dhcp() {
	DEBUG_ENTRY

	net::dhcp_start();

	network_store_save_dhcp(true);

	DEBUG_EXIT
}

void net_set_zeroconf() {
	DEBUG_ENTRY

	net::autoip_start();

	network_store_save_dhcp(false);

	DEBUG_EXIT
}
}  // namespace net
