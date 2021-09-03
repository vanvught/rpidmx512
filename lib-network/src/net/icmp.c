/**
 * @file icmp.c
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>

#include "net.h"
#include "net_packets.h"
#include "net_debug.h"

extern struct ip_info g_ip_info;
extern uint8_t g_mac_address[ETH_ADDR_LEN];

extern uint16_t net_chksum(void*, uint32_t);
extern void emac_eth_send(void*, int);

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

__attribute__((hot)) void icmp_handle(struct t_icmp *p_icmp) {
	_pcast32 src;
	DEBUG_ENTRY

	if (p_icmp->icmp.type == ICMP_TYPE_ECHO) {
		if (p_icmp->icmp.code == ICMP_CODE_ECHO) {
			// Ethernet
			memcpy(p_icmp->ether.dst, p_icmp->ether.src, ETH_ADDR_LEN);
			memcpy(p_icmp->ether.src, g_mac_address, ETH_ADDR_LEN);
			// IPv4
			p_icmp->ip4.id = (uint16_t)(~p_icmp->ip4.id);
			memcpy(p_icmp->ip4.dst, p_icmp->ip4.src, IPv4_ADDR_LEN);
			src.u32 = g_ip_info.ip.addr;
			memcpy(p_icmp->ip4.src, src.u8, IPv4_ADDR_LEN);
			p_icmp->ip4.chksum = 0;
			p_icmp->ip4.chksum = net_chksum((void *)&p_icmp->ip4, 20); //TODO
			// ICMP
			p_icmp->icmp.type = ICMP_TYPE_ECHO_REPLY;
			p_icmp->icmp.checksum = 0;
			p_icmp->icmp.checksum = net_chksum((void *)&p_icmp->ip4, (uint32_t)__builtin_bswap16(p_icmp->ip4.len));

			debug_dump(p_icmp, sizeof(struct ether_header) + __builtin_bswap16(p_icmp->ip4.len));

			emac_eth_send((void *)p_icmp, (int) (sizeof(struct ether_header) + __builtin_bswap16(p_icmp->ip4.len)));
		}
	}

	DEBUG_EXIT
}
