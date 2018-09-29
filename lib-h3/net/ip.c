/**
 * @file ip.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "net/net.h"

#include "net_packets.h"
#include "net_debug.h"

#if defined(DO_NET_CHKSUM)
 extern uint16_t net_chksum(void *, uint32_t);
#endif

 extern void udp_init(const uint8_t *, const struct ip_info  *);
extern void udp_set_ip(const struct ip_info  *);
extern void udp_handle(struct t_udp *);

extern void igmp_init(const uint8_t *, const struct ip_info  *);
extern void igmp_set_ip(const struct ip_info  *);
extern void igmp_handle(struct t_igmp *);

extern void icmp_init(const uint8_t *, const struct ip_info  *);
extern void icmp_set_ip(const struct ip_info  *);
extern void icmp_handle(struct t_icmp *);

void ip_set_ip(const struct ip_info *p_ip_info) {
	udp_set_ip(p_ip_info);
	igmp_set_ip(p_ip_info);
	icmp_set_ip(p_ip_info);
}

void ip_init(const uint8_t *mac_address, const struct ip_info *p_ip_info) {
	udp_init(mac_address, p_ip_info);
	igmp_init(mac_address, p_ip_info);
	icmp_init(mac_address, p_ip_info);
}

void ip_handle(struct t_ip4 *p_ip4) {
	if  (__builtin_expect((p_ip4->ip4.ver_ihl != 0x45), 0)) {
		if (p_ip4->ip4.proto == IPv4_PROTO_IGMP) {
			igmp_handle((struct t_igmp *) p_ip4);
		} else {
			DEBUG_PRINTF("p_ip4->ip4.ver_ihl=0x%x\n", p_ip4->ip4.ver_ihl);
		}
		return;
	}

#if defined(DO_NET_CHKSUM)
	uint16_t chksum;
	// Really needed, doesn't do EMAC this job?
	if ((chksum = net_chksum((void *) &p_ip4->ip4, sizeof(p_ip4->ip4))) != 0) {
		printf("chksum=%d\n", chksum);
		assert(0);
		DEBUG1_EXIT
		return;
	}
#endif

	switch (p_ip4->ip4.proto) {
	case IPv4_PROTO_UDP:
		udp_handle((struct t_udp *) p_ip4);
		break;
	case IPv4_PROTO_IGMP:
		igmp_handle((struct t_igmp *) p_ip4);
		break;
	case IPv4_PROTO_ICMP:
		icmp_handle((struct t_icmp *) p_ip4);
		break;
	default:
		DEBUG_PRINTF("proto %d not implemented", p_ip4->ip4.proto);
		break;
	}
}
