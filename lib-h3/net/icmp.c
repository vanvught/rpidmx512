/**
 * @file icmp.c
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

#include "util.h"

static struct t_icmp s_reply;

extern uint16_t net_chksum(void *, uint32_t);
extern void emac_eth_send(void *, int);

typedef union pcast32 {
		uint32_t u32;
		uint8_t u8[4];
} _pcast32;

void icmp_set_ip(const struct ip_info  *p_ip_info) {
	_pcast32 src;

	src.u32 = p_ip_info->ip.addr;
	memcpy(s_reply.ip4.src, src.u8, IPv4_ADDR_LEN);
}

void icmp_init(const uint8_t *mac_address, const struct ip_info  *p_ip_info) {
	// Ethernet
	memcpy(s_reply.ether.src, mac_address, ETH_ADDR_LEN);
	s_reply.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
	// IPv4
	s_reply.ip4.ver_ihl = 0x45; // TODO
	s_reply.ip4.tos = 0;
	s_reply.ip4.flags_froff = 0;
	s_reply.ip4.ttl = 64;
	s_reply.ip4.proto = IPv4_PROTO_ICMP;
	icmp_set_ip(p_ip_info);
	// ICMP
	s_reply.icmp.type = ICMP_TYPE_ECHO_REPLY;
	s_reply.icmp.code = ICMP_CODE_ECHO;
}

void icmp_handle(struct t_icmp *p_icmp) {
	DEBUG2_ENTRY

	if (p_icmp->icmp.type == ICMP_TYPE_ECHO) {
		if (p_icmp->icmp.code == ICMP_CODE_ECHO) {
			// Ethernet
			memcpy(s_reply.ether.dst, p_icmp->ether.src, ETH_ADDR_LEN);
			// IPv4
			s_reply.ip4.len = p_icmp->ip4.len;
			s_reply.ip4.id = ~p_icmp->ip4.id;
			memcpy(s_reply.ip4.dst, p_icmp->ip4.src, IPv4_ADDR_LEN);
			s_reply.ip4.chksum = 0;
			s_reply.ip4.chksum = net_chksum((void *)&s_reply.ip4, 20); //TODO
			// ICMP
			memcpy(s_reply.icmp.parameter, p_icmp->icmp.parameter, sizeof(s_reply.icmp.parameter));

			uint32_t payload_size = __builtin_bswap16(s_reply.ip4.len) + 8; //TODO

			memcpy(s_reply.icmp.payload, p_icmp->icmp.payload, payload_size);

			s_reply.icmp.checksum = 0;
			s_reply.icmp.checksum = net_chksum((void *)&s_reply.ip4, (uint32_t)__builtin_bswap16(p_icmp->ip4.len));

			debug_dump((void *)&s_reply, sizeof(struct ether_packet) + __builtin_bswap16(p_icmp->ip4.len));

			emac_eth_send((void *)&s_reply, sizeof(struct ether_packet) + __builtin_bswap16(p_icmp->ip4.len));
		}
	}

	DEBUG2_EXIT
}

