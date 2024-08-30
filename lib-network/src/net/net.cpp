/**
 * @file net.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#endif

#include <cstdint>
#include <cstring>

#include "../../config/net_config.h"

#include "net.h"
#include "net_private.h"

#include "netif.h"
#include "net/acd.h"
#include "net/dhcp.h"

#include "debug.h"

static struct net::acd::Acd s_acd;

namespace network {
__attribute__((weak)) void mdns_shutdown() {}
}  // namespace network

namespace net {
namespace globals {
uint32_t nBroadcastMask;
uint32_t nOnNetworkMask;
}  // namespace globals
#if defined (CONFIG_ENET_ENABLE_PTP)
void ptp_init();
void ptp_handle(const uint8_t *, const uint32_t);
#endif

void net_shutdown() {
	DEBUG_ENTRY

	net::netif_set_link_down();

	DEBUG_EXIT
}

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

void net_set_primary_ip(const ip4_addr_t ipaddr) {
	auto &netif = net::globals::netif_default;

	net::dhcp_release_and_stop();

	acd_add(&s_acd, primary_ip_conflict_callback);

	if (ipaddr.addr  == 0) {
		acd_start(&s_acd, netif.secondary_ip);
	} else {
		acd_start(&s_acd, ipaddr);
	}
}

void net_set_secondary_ip() {
	auto &netif = net::globals::netif_default;
	ip4_addr_t netmask;
	netmask.addr = 255;
	net::netif_set_addr(netif.secondary_ip, netmask, netif.secondary_ip);
}

void __attribute__((cold)) net_init(const net::Link link, ip4_addr_t ipaddr, ip4_addr_t netmask, ip4_addr_t gw, bool &bUseDhcp) {
	DEBUG_ENTRY
	globals::netif_default.secondary_ip.addr = 2
			+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[3])) << 8)
			+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[4])) << 16)
			+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[5])) << 24);

	net::arp_init();
	ip_init();

	if (net::Link::STATE_UP == link) {
		net::netif_set_flags(net::netif::NETIF_FLAG_LINK_UP);

		if (bUseDhcp) {
			dhcp_start();
		} else {
//			if (ipaddr.addr == 0) {
//				net_set_secondary_ip();
//			} else {
//				net::netif_set_addr(ipaddr, netmask, gw);
//			}
//			dhcp_inform();
			if (ipaddr.addr != 0) {
				net::netif_set_netmask(netmask);
				net::netif_set_gw(gw);
			}
			net_set_primary_ip(ipaddr);
		}
	} else {
		net::netif_clear_flags(net::netif::NETIF_FLAG_LINK_UP);
	}

#if defined (CONFIG_ENET_ENABLE_PTP)
	net::ptp_init();
#endif

	DEBUG_EXIT
}

__attribute__((hot)) void net_handle() {
	uint8_t *s_p;
	const auto nLength = emac_eth_recv(&s_p);

	if (__builtin_expect((nLength > 0), 0)) {
		const auto *const eth = reinterpret_cast<struct ether_header *>(s_p);

#if defined (CONFIG_ENET_ENABLE_PTP)
		if (eth->type == __builtin_bswap16(ETHER_TYPE_PTP)) {
			net::ptp_handle(const_cast<const uint8_t *>(s_p), nLength);
		} else
#endif
			if (eth->type == __builtin_bswap16(ETHER_TYPE_IPv4)) {
				ip_handle(reinterpret_cast<struct t_ip4 *>(s_p));
			} else if (eth->type == __builtin_bswap16(ETHER_TYPE_ARP)) {
				net::arp_handle(reinterpret_cast<struct t_arp *>(s_p));
			} else {
				DEBUG_PRINTF("type %04x is not implemented", __builtin_bswap16(eth->type));
			}

		emac_free_pkt();
	}
}
}  // namespace net
