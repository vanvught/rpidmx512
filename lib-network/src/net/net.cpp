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
#include "net_private.h"

#include "net/net.h"
#include "net/netif.h"
#include "net/dhcp.h"
#include "net/tcp.h"
#include "net/igmp.h"
#if defined (CONFIG_NET_ENABLE_NTP_CLIENT) || defined (CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
# include "net/apps/ntp_client.h"
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
# include "net/apps/mdns.h"
#endif

#include "debug.h"

namespace net {
namespace globals {
uint32_t nBroadcastMask;
uint32_t nOnNetworkMask;
}  // namespace globals

#if defined (CONFIG_NET_ENABLE_PTP)
 __attribute__((weak)) void ptp_init() {}
 /* Can only be used for PTP level 2 messages */
 __attribute__((weak)) void ptp_handle([[maybe_unused]] const uint8_t *, [[maybe_unused]] const uint32_t) {}
#endif

void net_link_down() {
#if !defined(CONFIG_NET_APPS_NO_MDNS)
	mdns_send_announcement(0);
#endif
#if defined (ENABLE_HTTPD)
 	tcp_shutdown();
 #endif
	igmp_shutdown();
	dhcp_release_and_stop();
}

void net_shutdown() {
	DEBUG_ENTRY

	net::netif_set_link_down();

	DEBUG_EXIT
}

void __attribute__((cold)) net_init(const net::Link link, ip4_addr_t ipaddr, ip4_addr_t netmask, ip4_addr_t gw, bool &bUseDhcp) {
	DEBUG_ENTRY
	globals::netif_default.secondary_ip.addr = 2
			+ ((static_cast<uint32_t>(static_cast<uint8_t>(globals::netif_default.hwaddr[3] + 0xFF + 0xFF))) << 8)
			+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[4])) << 16)
			+ ((static_cast<uint32_t>(globals::netif_default.hwaddr[5])) << 24);

	net::arp_init();
	ip_init();

	if (net::Link::STATE_UP == link) {
		net::netif_set_flags(net::netif::NETIF_FLAG_LINK_UP);

		if (bUseDhcp) {
			dhcp_start();
		} else {
			if (ipaddr.addr != 0) {
				net::netif_set_netmask(netmask);
				net::netif_set_gw(gw);
			}
			net_set_primary_ip(ipaddr.addr);
		}
	} else {
		net::netif_clear_flags(net::netif::NETIF_FLAG_LINK_UP);
	}

#if defined (CONFIG_NET_ENABLE_PTP)
	net::ptp_init();
#endif
#if defined (CONFIG_NET_ENABLE_NTP_CLIENT)
	ntp_client_init();
#endif
#if defined (CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
	ptp_ntp_init();
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
	mdns_init();
#endif
	DEBUG_EXIT
}

__attribute__((hot)) void net_handle() {
	uint8_t *s_p;
	const auto nLength = emac_eth_recv(&s_p);

	if (__builtin_expect((nLength > 0), 0)) {
		const auto *const eth = reinterpret_cast<struct ether_header *>(s_p);

#if defined (CONFIG_NET_ENABLE_PTP)
		if (eth->type == __builtin_bswap16(ETHER_TYPE_PTP)) {
			net::ptp_handle(const_cast<const uint8_t *>(s_p), nLength);
		} else
#endif
			if (eth->type == __builtin_bswap16(ETHER_TYPE_IPv4)) {
				auto *p_ip4 = reinterpret_cast<struct t_ip4 *>(s_p);
				if  (__builtin_expect((p_ip4->ip4.ver_ihl != 0x45), 0)) {
					if (p_ip4->ip4.proto == IPv4_PROTO_IGMP) {
						igmp_handle(reinterpret_cast<struct t_igmp *>(p_ip4));
					} else {
						DEBUG_PRINTF("p_ip4->ip4.ver_ihl=0x%x", p_ip4->ip4.ver_ihl);
					}
				} else {
					switch (p_ip4->ip4.proto) {
					case IPv4_PROTO_UDP:
						udp_handle(reinterpret_cast<struct t_udp *>(p_ip4));
						// NOTE: emac_free_pkt(); is done in udp_handle
						return;
						break;
					case IPv4_PROTO_IGMP:
						igmp_handle(reinterpret_cast<struct t_igmp *>(p_ip4));
						break;
					case IPv4_PROTO_ICMP:
						icmp_handle(reinterpret_cast<struct t_icmp *>(p_ip4));
						break;
#if defined (ENABLE_HTTPD)
					case IPv4_PROTO_TCP:
						tcp_input(reinterpret_cast<struct t_tcp *>(p_ip4));
						tcp_run();
						break;
#endif
					default:
						DEBUG_PRINTF("proto %d not implemented", p_ip4->ip4.proto);
						break;
					}
				}
			} else if (eth->type == __builtin_bswap16(ETHER_TYPE_ARP)) {
				net::arp_handle(reinterpret_cast<struct t_arp *>(s_p));
			} else {
				DEBUG_PRINTF("type %04x is not implemented", __builtin_bswap16(eth->type));
			}

		emac_free_pkt();
	}
}
}  // namespace net
