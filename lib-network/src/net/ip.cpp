/**
 * @file ip.cpp
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

#if defined (DEBUG_NET_IP)
# undef NDEBUG
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>

#include "../../config/net_config.h"

#include "net.h"
#include "net_private.h"

#include "debug.h"

namespace net {
void ip_set_ip() {
	udp_set_ip();
	igmp_set_ip();
}

void __attribute__((cold)) ip_init() {
	DEBUG_ENTRY

	udp_init();
	igmp_init();
#if defined (ENABLE_HTTPD)
	tcp_init();
#endif

	DEBUG_EXIT
}

__attribute__((hot)) void ip_handle(struct t_ip4 *p_ip4) {
	if  (__builtin_expect((p_ip4->ip4.ver_ihl != 0x45), 0)) {
		if (p_ip4->ip4.proto == IPv4_PROTO_IGMP) {
			igmp_handle(reinterpret_cast<struct t_igmp *>(p_ip4));
		} else {
			DEBUG_PRINTF("p_ip4->ip4.ver_ihl=0x%x", p_ip4->ip4.ver_ihl);
		}
		return;
	}

	switch (p_ip4->ip4.proto) {
	case IPv4_PROTO_UDP:
		udp_handle(reinterpret_cast<struct t_udp *>(p_ip4));
		break;
	case IPv4_PROTO_IGMP:
		igmp_handle(reinterpret_cast<struct t_igmp *>(p_ip4));
		break;
	case IPv4_PROTO_ICMP:
		icmp_handle(reinterpret_cast<struct t_icmp *>(p_ip4));
		break;
#if defined (ENABLE_HTTPD)
	case IPv4_PROTO_TCP:
		tcp_handle(reinterpret_cast<struct t_tcp *>(p_ip4));
		tcp_run();
		break;
#endif
	default:
		DEBUG_PRINTF("proto %d not implemented", p_ip4->ip4.proto);
		break;
	}
}
}  // namespace net
