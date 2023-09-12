/**
 * @file igmp.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "net.h"
#include "net_private.h"

#include "../../config/net_config.h"

/*
 * https://www.rfc-editor.org/rfc/rfc2236.html
 * Internet Group Management Protocol, Version 2
 */

enum State {
	NON_MEMBER, DELAYING_MEMBER, IDLE_MEMBER
};

struct t_group_info {
	uint32_t nGroupAddress;
	uint16_t nTimer;		// 1/10 seconds
	State state;
};

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static struct t_igmp s_report SECTION_NETWORK ALIGNED;
static struct t_igmp s_leave SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] SECTION_NETWORK ALIGNED;
static struct t_group_info s_groups[IGMP_MAX_JOINS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;

namespace net {
namespace globals {
extern struct IpInfo ipInfo;
extern uint8_t macAddress[ETH_ADDR_LEN];
}  // namespace globals
}  // namespace net

static void _send_report(uint32_t nGroupAddress);

void igmp_set_ip() {
	_pcast32 src;

	src.u32 = net::globals::ipInfo.ip.addr;

	memcpy(s_report.ip4.src, src.u8, IPv4_ADDR_LEN);
	memcpy(s_leave.ip4.src, src.u8, IPv4_ADDR_LEN);
}

void __attribute__((cold)) igmp_init() {
	igmp_set_ip();

	s_multicast_mac[0] = 0x01;
	s_multicast_mac[1] = 0x00;
	s_multicast_mac[2] = 0x5E;

	// Ethernet
	memcpy(s_report.ether.src, net::globals::macAddress, ETH_ADDR_LEN);
	s_report.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);

	// IPv4
	s_report.ip4.ver_ihl = 0x46; // TODO
	s_report.ip4.tos = 0;
	s_report.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_report.ip4.ttl = 1;
	s_report.ip4.proto = IPv4_PROTO_IGMP;
	s_report.ip4.len = __builtin_bswap16(IPv4_IGMP_REPORT_HEADERS_SIZE);
	// IPv4 options
	s_report.igmp.report.ip4_options = 0x00000494; // TODO

	// IGMP
	s_report.igmp.report.igmp.type = IGMP_TYPE_REPORT;
	s_report.igmp.report.igmp.max_resp_time = 0;

	// Ethernet
	s_leave.ether.dst[0] = 0x01;
	s_leave.ether.dst[1] = 0x00;
	s_leave.ether.dst[2] = 0x5E;
	s_leave.ether.dst[3] = 0x00;
	s_leave.ether.dst[4] = 0x00;
	s_leave.ether.dst[5] = 0x02;
	memcpy(s_leave.ether.src, net::globals::macAddress, ETH_ADDR_LEN);
	s_leave.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);

	// IPv4
	s_leave.ip4.ver_ihl = 0x46; // TODO
	s_leave.ip4.tos = 0;
	s_leave.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_leave.ip4.ttl = 1;
	s_leave.ip4.proto = IPv4_PROTO_IGMP;
	s_leave.ip4.len = __builtin_bswap16(IPv4_IGMP_REPORT_HEADERS_SIZE);
	s_leave.ip4.dst[0] = 0xE0; // 224
	s_leave.ip4.dst[1] = 0x00; // 0
	s_leave.ip4.dst[2] = 0x00; // 0
	s_leave.ip4.dst[3] = 0x02; // 2
	// IPv4 options
	s_leave.igmp.report.ip4_options = 0x00000494; // TODO

	// IGMP
	s_leave.igmp.report.igmp.type = IGMP_TYPE_LEAVE;
	s_leave.igmp.report.igmp.max_resp_time = 0;

	/*
	 * https://tldp.org/HOWTO/Multicast-HOWTO-2.html
	 * 224.0.0.1 is the all-hosts group. If you ping that group,
	 * all multicast capable hosts on the network should answer,
	 * as every multicast capable host must join that group
	 * at start-up on all it's multicast capable interfaces.
	 */
	_send_report(0x010000e0);
}

void __attribute__((cold)) igmp_shutdown() {
	DEBUG_ENTRY

	for (auto& group : s_groups) {
		if (group.nGroupAddress != 0) {
			DEBUG_PRINTF(IPSTR, IP2STR(group.nGroupAddress));

			igmp_leave(group.nGroupAddress);
		}
	}

	DEBUG_EXIT
}

static void _send_report(const uint32_t nGroupAddress) {
	DEBUG_ENTRY
	_pcast32 multicast_ip;

	multicast_ip.u32 = nGroupAddress;

	s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
	s_multicast_mac[4] = multicast_ip.u8[2];
	s_multicast_mac[5] = multicast_ip.u8[3];

	DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(nGroupAddress),MAC2STR(s_multicast_mac));

	// Ethernet
	memcpy(s_report.ether.dst, s_multicast_mac, ETH_ADDR_LEN);
	// IPv4
	s_report.ip4.id = s_id;
	memcpy(s_report.ip4.dst, multicast_ip.u8, IPv4_ADDR_LEN);
	s_report.ip4.chksum = 0;
	s_report.ip4.chksum = net_chksum(reinterpret_cast<void *>(&s_report.ip4), 24); //TODO
	// IGMP
	memcpy(s_report.igmp.report.igmp.group_address, multicast_ip.u8, IPv4_ADDR_LEN);
	s_report.igmp.report.igmp.checksum = 0;
	s_report.igmp.report.igmp.checksum = net_chksum(reinterpret_cast<void *>(&s_report.ip4), IPv4_IGMP_REPORT_HEADERS_SIZE);

	emac_eth_send(reinterpret_cast<void *>(&s_report), IGMP_REPORT_PACKET_SIZE);

	s_id++;

	DEBUG_EXIT
}

static void _send_leave(const uint32_t nGroupAddress) {
	DEBUG_ENTRY
	_pcast32 multicast_ip;

	multicast_ip.u32 = nGroupAddress;

	DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(nGroupAddress), MAC2STR(s_multicast_mac));

	// IPv4
	s_leave.ip4.id = s_id;
	s_leave.ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
	s_leave.ip4.chksum = net_chksum(reinterpret_cast<void *>(&s_leave.ip4), 24); //TODO
#endif
	// IGMP
	memcpy(s_leave.igmp.report.igmp.group_address, multicast_ip.u8, IPv4_ADDR_LEN);
	s_leave.igmp.report.igmp.checksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
	s_leave.igmp.report.igmp.checksum = net_chksum(reinterpret_cast<void *>(&s_leave.ip4), IPv4_IGMP_REPORT_HEADERS_SIZE);
#endif

	emac_eth_send(reinterpret_cast<void *>(&s_leave), IGMP_REPORT_PACKET_SIZE);

	s_id++;

	DEBUG_EXIT
}

__attribute__((hot)) void igmp_handle(struct t_igmp *p_igmp) {
	DEBUG_ENTRY

	if ((p_igmp->ip4.ver_ihl == 0x45) && (p_igmp->igmp.igmp.type == IGMP_TYPE_QUERY)) {
		DEBUG_PRINTF(IPSTR, p_igmp->ip4.dst[0], p_igmp->ip4.dst[1], p_igmp->ip4.dst[2], p_igmp->ip4.dst[3]);

		auto isGeneralRequest = false;

		_pcast32 igmp_generic_address;
		igmp_generic_address.u32 = 0x010000e0;

		if (memcmp(p_igmp->ip4.dst, igmp_generic_address.u8, 4) == 0) {
			isGeneralRequest = true;
		}

		for (auto& group : s_groups) {
			if (group.nGroupAddress == 0) {
				continue;
			}

			_pcast32 group_address;
			group_address.u32 = group.nGroupAddress;

			if (isGeneralRequest || ( memcmp(p_igmp->ip4.dst, group_address.u8, IPv4_ADDR_LEN) == 0)) {
				if (group.state == DELAYING_MEMBER) {
					if (p_igmp->igmp.igmp.max_resp_time  < group.nTimer) {
						group.nTimer = (1 + p_igmp->igmp.igmp.max_resp_time / 2);
					}
				} else { // s_groups[s_joins_allowed_index].state == IDLE_MEMBER
					group.state = DELAYING_MEMBER;
					group.nTimer = (1 + p_igmp->igmp.igmp.max_resp_time / 2);
				}
			}
		}
	}

	DEBUG_EXIT
}

void igmp_timer() {
	for (auto& group : s_groups) {
		if ((group.state == DELAYING_MEMBER) && (group.nTimer > 0)) {
			group.nTimer--;

			if (group.nTimer == 0) {
				_send_report(group.nGroupAddress);
				group.state = IDLE_MEMBER;
			}
		}
	}
}

// --> Public

void igmp_join(uint32_t nGroupAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR, IP2STR(nGroupAddress));

	if ((nGroupAddress & 0xE0) != 0xE0) {
		DEBUG_ENTRY
		return;
	}

	for (int i = 0; i < IGMP_MAX_JOINS_ALLOWED; i++) {
		if (s_groups[i].nGroupAddress == nGroupAddress) {
			DEBUG_EXIT
			return;
		}

		if (s_groups[i].nGroupAddress == 0) {
			s_groups[i].nGroupAddress = nGroupAddress;
			s_groups[i].state = DELAYING_MEMBER;
			s_groups[i].nTimer = 2; // TODO

			_send_report(nGroupAddress);

			DEBUG_EXIT
			return;
		}
	}


#ifndef NDEBUG
	console_error("igmp_join\n");
#endif
	DEBUG_ENTRY
}

void igmp_leave(uint32_t nGroupAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR, IP2STR(nGroupAddress));

	for (auto& group : s_groups) {
		if (group.nGroupAddress == nGroupAddress) {
			_send_leave(group.nGroupAddress);

			group.nGroupAddress = 0;
			group.state = NON_MEMBER;
			group.nTimer = 0;

			DEBUG_EXIT
			return;
		}
	}

#ifndef NDEBUG
	console_error("igmp_leave: ");
	printf(IPSTR "\n", IP2STR(nGroupAddress));
#endif
	DEBUG_EXIT
}

// <---
