/**
 * @file arp.c
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
#include <assert.h>

#include "net/net.h"

#include "net_packets.h"
#include "net_debug.h"

#include "util.h"

extern void arp_cache_init(void);
extern void arp_cache_update(uint8_t *, uint32_t);

extern void emac_eth_send(void *, int);

static struct t_arp s_arp_announce ALIGNED;
static struct t_arp s_arp_request ALIGNED;
static struct t_arp s_arp_reply ALIGNED;

static void arp_announce(void) {
	DEBUG_ENTRY

	if(s_arp_announce.arp.sender_ip == 0) {
		DEBUG_EXIT
		return;
	}

	debug_dump((void *)&s_arp_announce, sizeof(struct t_arp));

	emac_eth_send((void *)&s_arp_announce, sizeof(struct t_arp));

	DEBUG_EXIT
}

static void arp_handle_request(struct t_arp *p_arp) {
	DEBUG2_ENTRY

	// Ethernet header
	memcpy(s_arp_reply.ether.dst, p_arp->ether.src, ETH_ADDR_LEN);

	// ARP Header
	memcpy(s_arp_reply.arp.target_mac, p_arp->arp.sender_mac, ETH_ADDR_LEN);
	s_arp_reply.arp.target_ip = p_arp->arp.sender_ip;

	DEBUG_PRINTF(IPSTR, IP2STR(p_arp->arp.sender_ip));

	emac_eth_send((void *)&s_arp_reply, sizeof(struct t_arp));

	DEBUG2_EXIT
}

static void arp_handle_reply(struct t_arp *p_arp) {
	DEBUG2_ENTRY

	arp_cache_update(p_arp->arp.sender_mac, p_arp->arp.sender_ip);

	DEBUG2_EXIT
}

void arp_init(const uint8_t *mac_address, const struct ip_info  *p_ip_info) {
	arp_cache_init();

	const uint32_t local_ip = p_ip_info->ip.addr;

	// ARP Announce
	// Ethernet header
	memcpy(s_arp_announce.ether.src, mac_address, ETH_ADDR_LEN);
	memset(s_arp_announce.ether.dst, 0xFF , ETH_ADDR_LEN);
	s_arp_announce.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);
	// ARP Header
	s_arp_announce.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_announce.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_announce.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_announce.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_announce.arp.opcode = __builtin_bswap16(ARP_OPCODE_RQST);

	memcpy(s_arp_announce.arp.sender_mac, mac_address, ETH_ADDR_LEN);
	s_arp_announce.arp.sender_ip = local_ip;
	memset(s_arp_announce.arp.target_mac, 0x00, ETH_ADDR_LEN);
	s_arp_announce.arp.target_ip = local_ip;


	// ARP Request template
	// Ethernet header
	memcpy(s_arp_request.ether.src, mac_address, ETH_ADDR_LEN);
	memset(s_arp_request.ether.dst, 0xFF , ETH_ADDR_LEN);
	s_arp_request.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);
	// ARP Header
	s_arp_request.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_request.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_request.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_request.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_request.arp.opcode = __builtin_bswap16(ARP_OPCODE_RQST);

	memcpy(s_arp_request.arp.sender_mac, mac_address, ETH_ADDR_LEN);
	s_arp_request.arp.sender_ip = local_ip;
	memset(s_arp_request.arp.target_mac, 0x00, ETH_ADDR_LEN);

	// ARP Reply Template
	// Ethernet header
	memcpy(s_arp_reply.ether.src, mac_address, ETH_ADDR_LEN);
	s_arp_reply.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);
	// ARP Header
	s_arp_reply.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_reply.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_reply.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_reply.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_reply.arp.opcode = __builtin_bswap16(ARP_OPCODE_REPLY);

	memcpy(s_arp_reply.arp.sender_mac, mac_address, ETH_ADDR_LEN);
	s_arp_reply.arp.sender_ip = local_ip;

	arp_announce();
}

void arp_send_request(uint32_t ip) {
	DEBUG2_ENTRY

	// ARP Header
	s_arp_request.arp.target_ip = ip;

	DEBUG_PRINTF(IPSTR, IP2STR(ip));

	emac_eth_send((void *)&s_arp_request, sizeof(struct t_arp));

	DEBUG2_EXIT
}

void arp_handle(struct t_arp *p_arp) {
	DEBUG1_ENTRY

	const uint16_t opcode = __builtin_bswap16(p_arp->arp.opcode);

	switch (opcode) {
		case ARP_OPCODE_RQST:
			arp_handle_request(p_arp);
			break;
		case ARP_OPCODE_REPLY:
			arp_handle_reply(p_arp);
			break;
		default:
			DEBUG_PRINTF("opcode %04x not handled", opcode);
			break;
	}

	DEBUG1_EXIT
}
