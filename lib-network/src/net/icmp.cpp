/**
 * @file icmp.cpp
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

#if defined (DEBUG_NET_ICMP)
# undef NDEBUG
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>

#include "../../config/net_config.h"

#include "net_memcpy.h"
#include "net_private.h"

#include "net/protocol/icmp.h"

#include "debug.h"

namespace net {
__attribute__((hot)) void icmp_handle(struct t_icmp *p_icmp) {
	if (p_icmp->icmp.type == ICMP_TYPE_ECHO) {
		if (p_icmp->icmp.code == ICMP_CODE_ECHO) {
			// Ethernet
			std::memcpy(p_icmp->ether.dst, p_icmp->ether.src, ETH_ADDR_LEN);
			std::memcpy(p_icmp->ether.src, globals::netif_default.hwaddr, ETH_ADDR_LEN);
			// IPv4
			p_icmp->ip4.id = static_cast<uint16_t>(~p_icmp->ip4.id);

			const auto nIpDestination = net::memcpy_ip(p_icmp->ip4.dst);

			std::memcpy(p_icmp->ip4.dst, p_icmp->ip4.src, IPv4_ADDR_LEN);

			if (nIpDestination == globals::netif_default.secondary_ip.addr) {
				net::memcpy_ip(p_icmp->ip4.src, globals::netif_default.secondary_ip.addr);
			} else {
				net::memcpy_ip(p_icmp->ip4.src, globals::netif_default.ip.addr);
			}

			p_icmp->ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
			p_icmp->ip4.chksum = net_chksum(reinterpret_cast<void *>(&p_icmp->ip4), 20); //TODO
#endif
			// ICMP
			p_icmp->icmp.type = ICMP_TYPE_ECHO_REPLY;
			p_icmp->icmp.checksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
			p_icmp->icmp.checksum = net_chksum(reinterpret_cast<void *>(&p_icmp->ip4), static_cast<uint32_t>(__builtin_bswap16(p_icmp->ip4.len)));
#endif
			emac_eth_send(reinterpret_cast<void *>(p_icmp), static_cast<uint32_t>(sizeof(struct ether_header) + __builtin_bswap16(p_icmp->ip4.len)));
		}
	}
}
}  // namespace net
