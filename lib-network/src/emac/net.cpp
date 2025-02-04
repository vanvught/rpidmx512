/**
 * @file net.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_NET_NET)
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
#include <cstring>

#include "../../config/net_config.h"

#include "net.h"

#include "net/net.h"
#include "net/netif.h"
#include "net/autoip.h"
#include "net/dhcp.h"
#include "net/tcp.h"
#include "net/igmp.h"
#if defined (CONFIG_NET_ENABLE_NTP_CLIENT) || defined (CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
# include "net/apps/ntp_client.h"
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
# include "net/apps/mdns.h"
#endif

#include "network_store.h"

#include "debug.h"

namespace net {
namespace globals {
uint32_t nBroadcastMask;
uint32_t nOnNetworkMask;
}  // namespace globals

void net_shutdown() {
	DEBUG_ENTRY

#if !defined(CONFIG_NET_APPS_NO_MDNS)
	mdns_stop();
#endif
	net::igmp_shutdown();
	net::netif_set_link_down();

	DEBUG_EXIT
}

static struct net::acd::Acd s_acd;

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

void netif_set(const net::Link link, ip4_addr_t ipaddr, ip4_addr_t netmask, ip4_addr_t gw, bool bUseDhcp) {
	DEBUG_ENTRY

	globals::netif_default.secondary_ip.addr = 2
	+ ((static_cast<uint32_t>(static_cast<uint8_t>(globals::netif_default.hwaddr[3] + 0xFF + 0xFF))) << 8)
	+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[4])) << 16)
	+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[5])) << 24);

	if (!bUseDhcp) {
		acd_add(&s_acd, primary_ip_conflict_callback);

		if (ipaddr.addr != 0) {
			net::netif_set_netmask(netmask);
			net::netif_set_gw(gw);
		}
	}

	if (net::Link::STATE_UP == link) {
		net::netif_set_flags(net::netif::NETIF_FLAG_LINK_UP);
	} else {
		net::netif_clear_flags(net::netif::NETIF_FLAG_LINK_UP);
	}

	if (bUseDhcp) {
		dhcp_start();
	} else {
		if (ipaddr.addr == 0) {
			acd_start(&s_acd, globals::netif_default.secondary_ip);
		} else {
			acd_start(&s_acd, ipaddr);
		}
	}

	DEBUG_EXIT
}
}  // namespace net
