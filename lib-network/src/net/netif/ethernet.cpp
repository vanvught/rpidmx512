/**
 * @file ethernet.cpp
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

#if defined (DEBUG_NET_NETIF)
# undef NDEBUG
#endif

#include <cstdint>

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>

#include "net_private.h"
#include "net_memcpy.h"
#include "net/igmp.h"
#include "net/protocol/ethernet.h"

namespace net {
#if defined (CONFIG_NET_ENABLE_PTP)
/* Can only be used for PTP level 2 messages */
__attribute__((weak)) void ptp_handle([[maybe_unused]] const uint8_t *, [[maybe_unused]] const uint32_t) {}
#endif

void ethernet_input(const uint8_t *pBuffer, [[maybe_unused]] const uint32_t nLength) {
	const auto *const eth = reinterpret_cast<const struct ether_header *>(pBuffer);

	switch (eth->type) {
#if defined (CONFIG_NET_ENABLE_PTP)
	case __builtin_bswap16(ETHER_TYPE_PTP):
		net::ptp_handle(const_cast<const uint8_t *>(pBuffer), nLength);
		break;
#endif
	case __builtin_bswap16(ETHER_TYPE_IPv4): {
		auto *p_ip4 = reinterpret_cast<const struct t_ip4 *>(pBuffer);

		DEBUG_PRINTF(IPSTR " " IPSTR,
				p_ip4->ip4.dst[0],p_ip4->ip4.dst[1],p_ip4->ip4.dst[2],p_ip4->ip4.dst[3],
				p_ip4->ip4.src[0],p_ip4->ip4.src[1],p_ip4->ip4.src[2],p_ip4->ip4.src[3]);

		if ((eth->dst[0] == net::ETH_IP4_MULTICAST_ADDR_0) && (eth->dst[1] == net::ETH_IP4_MULTICAST_ADDR_1) && (eth->dst[2] == net::ETH_IP4_MULTICAST_ADDR_2)) {
			if (!igmp_lookup_group(memcpy_ip(p_ip4->ip4.dst))) {
				emac_free_pkt();
				DEBUG_PUTS("IGMP not for us");
				return;
			}
		}

		switch (p_ip4->ip4.proto) {
		case IPv4_PROTO_UDP:
			udp_input(reinterpret_cast<const struct t_udp *>(p_ip4));
			// NOTE: emac_free_pkt(); is done in udp_handle
			return;
			break;
		case IPv4_PROTO_IGMP:
			igmp_input(reinterpret_cast<const struct t_igmp *>(p_ip4));
			break;
		case IPv4_PROTO_ICMP:
			icmp_input(const_cast<struct t_icmp *>(reinterpret_cast<const struct t_icmp *>(p_ip4)));
			break;
#if defined (ENABLE_HTTPD)
		case IPv4_PROTO_TCP:
			tcp_input(const_cast<struct t_tcp *>(reinterpret_cast<const struct t_tcp *>(p_ip4)));
			tcp_run();
			break;
#endif
		default:
			break;
		}
	}
		break;
	case __builtin_bswap16(ETHER_TYPE_ARP):
		net::etharp_input(reinterpret_cast<const struct t_arp *>(pBuffer));
		break;
	default:
		DEBUG_PRINTF("type %04x is not implemented", __builtin_bswap16(eth->type));
		break;
	}

	emac_free_pkt();
}
}  // namespace net
