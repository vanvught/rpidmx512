/**
 * @file igmp.c
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
#include <stdbool.h>

#include "net/net.h"

#include "net_packets.h"
#include "net_debug.h"

#include "util.h"

extern uint16_t net_chksum(void *, uint32_t);
extern void emac_eth_send(void *, int);

#define MAX_JOINS_ALLOWED	4

typedef enum s_state {
	NON_MEMBER = 0,
	DELAYING_MEMBER,
	IDLE_MEMBER
} _state;

struct t_group_info {
	uint32_t group_address;
	uint8_t timer;		// 1/10 seconds
	_state state;
};

typedef union pcast32 {
		uint32_t u32;
		uint8_t u8[4];
} _pcast32;

static struct t_igmp s_report;
static uint8_t s_multicast_mac[ETH_ADDR_LEN];
static struct t_group_info s_groups[MAX_JOINS_ALLOWED] ALIGNED;
static uint8_t s_joins_allowed_index = 0;
static uint16_t s_id = 0;

void igmp_set_ip(const struct ip_info  *p_ip_info) {
	_pcast32 src;

	src.u32 = p_ip_info->ip.addr;
	memcpy(s_report.ip4.src, src.u8, IPv4_ADDR_LEN);
}

void igmp_init(uint8_t *mac_address, const struct ip_info  *p_ip_info) {
	uint16_t i;

	for (i = 0; i < MAX_JOINS_ALLOWED ; i++) {
		memset(&s_groups[i], 0, sizeof(struct t_group_info));
	}

	s_multicast_mac[0] = 0x01;
	s_multicast_mac[1] = 0x00;
	s_multicast_mac[2] = 0x5E;

	// Ethernet
	memcpy(s_report.ether.src, mac_address, ETH_ADDR_LEN);
	s_report.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
	// IPv4
	s_report.ip4.ver_ihl = 0x46; // TODO
	s_report.ip4.tos = 0;
	s_report.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_report.ip4.ttl = 1;
	s_report.ip4.proto = IPv4_PROTO_IGMP;
	s_report.ip4.len = __builtin_bswap16(IPv4_IGMP_REPORT_HEADERS_SIZE);
	igmp_set_ip(p_ip_info);
	// IPv4 options
	s_report.igmp.report.ip4_options = 0x00000494; // TODO
	// IGMP
	s_report.igmp.report.igmp.type = IGMP_TYPE_REPORT;
	s_report.igmp.report.igmp.max_resp_time = 0;
}

static void _send_report(uint32_t group_address) {
	DEBUG2_ENTRY
	_pcast32 multicast_ip;

	multicast_ip.u32 = group_address;

	s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
	s_multicast_mac[4] = multicast_ip.u8[2];
	s_multicast_mac[5] = multicast_ip.u8[3];

	DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(group_address),MAC2STR(s_multicast_mac));

	// Ethernet
	memcpy(s_report.ether.dst, s_multicast_mac, ETH_ADDR_LEN);
	// IPv4
	s_report.ip4.id = s_id;
	memcpy(s_report.ip4.dst, multicast_ip.u8, IPv4_ADDR_LEN);
	s_report.ip4.chksum = 0;
	s_report.ip4.chksum = net_chksum((void *)&s_report.ip4, 24); //TODO
	// IGMP
	memcpy(s_report.igmp.report.igmp.group_address, multicast_ip.u8, IPv4_ADDR_LEN);
	s_report.igmp.report.igmp.checksum = 0;
	s_report.igmp.report.igmp.checksum = net_chksum((void *)&s_report.ip4, (uint32_t)IPv4_IGMP_REPORT_HEADERS_SIZE);

	debug_dump((void *)&s_report, IGMP_REPORT_PACKET_SIZE);

	emac_eth_send((void *)&s_report, IGMP_REPORT_PACKET_SIZE);

	s_id++;

	DEBUG2_EXIT
}

static void _send_leave(uint32_t group_address) {
	//TODO static void _send_leave(uint32_t group_address)
}


void igmp_handle(struct t_igmp *p_igmp) {
	DEBUG2_ENTRY

	uint16_t i;
	_pcast32 igmp_generic_address;
	_pcast32 group_address;

	if ((p_igmp->ip4.ver_ihl == 0x45) && (p_igmp->igmp.igmp.type == IGMP_TYPE_QUERY)) {
		DEBUG_PRINTF(IPSTR, p_igmp->ip4.dst[0], p_igmp->ip4.dst[1], p_igmp->ip4.dst[2], p_igmp->ip4.dst[3]);

		bool  is_general_request = false;

		igmp_generic_address.u32 = 0x010000e0;

		if (memcmp(p_igmp->ip4.dst, igmp_generic_address.u8, 4) == 0) {
			is_general_request = true;
		}

		for (i = 0; i < s_joins_allowed_index; i++) {
			group_address.u32 = s_groups[i].group_address;
			if (is_general_request || ( memcmp(p_igmp->ip4.dst, group_address.u8, IPv4_ADDR_LEN) == 0)) {
				if (s_groups[i].state == DELAYING_MEMBER) {
					if (p_igmp->igmp.igmp.max_resp_time  < s_groups[i].timer) {
						s_groups[i].timer = 1 + p_igmp->igmp.igmp.max_resp_time / 2;
					}
				} else { // s_groups[s_joins_allowed_index].state == IDLE_MEMBER
					s_groups[i].state = DELAYING_MEMBER;
					s_groups[i].timer = 1 + p_igmp->igmp.igmp.max_resp_time / 2;
				}
			}
		}
	}

	DEBUG2_EXIT
}

void igmp_timer(void) {
	uint16_t i;
	// TODO optimize ?
	for (i = 0; i < MAX_JOINS_ALLOWED ; i++) {

		if ((s_groups[i].state == DELAYING_MEMBER) && (s_groups[i].timer > 0)) {
			s_groups[i].timer--;

			if (s_groups[i].timer == 0) {
				_send_report(s_groups[i].group_address);
				s_groups[i].state = IDLE_MEMBER;
			}
		}
	}
}

// --> Public

int igmp_join(uint32_t group_address) {
	uint16_t i;

	if ((group_address& 0xE0) != 0xE0) {
		return -1;
	}

	if (s_joins_allowed_index == MAX_JOINS_ALLOWED) {
		return -2;
	}

	for (i = 0; i < s_joins_allowed_index; i++) {
		if (s_groups[i].group_address == group_address) {
			return i;
		}
	}

	const int current_index = s_joins_allowed_index;

	s_groups[s_joins_allowed_index].group_address = group_address;
	s_groups[s_joins_allowed_index].state = DELAYING_MEMBER;
	s_groups[s_joins_allowed_index].timer = 2; // TODO

	s_joins_allowed_index++;

	_send_report(group_address);

	return current_index;
}

int igmp_leave(uint32_t group_address) {
	uint16_t i;

	for (i = 0; i < MAX_JOINS_ALLOWED; i++) {
		if (s_groups[i].group_address == group_address) {
			break;
		}
	}

	if (i == MAX_JOINS_ALLOWED) {
		DEBUG2_EXIT
		return -1;
	}

	_send_leave(s_groups[i].group_address);

	s_groups[i].group_address = 0;
	s_groups[i].state = NON_MEMBER;
	s_groups[i].timer = 0;

	return 0;
}

// <---
