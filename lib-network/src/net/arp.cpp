/**
 * @file arp.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cassert>

#include "net.h"
#include "net_private.h"
#include "net_memcpy.h"
#include "../../config/net_config.h"

#include "hardware.h"

namespace net {
namespace arp {
enum class RequestType {
	REQUEST, PROBE, ANNNOUNCEMENT
};
}  // namespace arp
}  // namespace net

static struct t_arp s_arp_request ALIGNED ;
static struct t_arp s_arp_reply ALIGNED;
static net::arp::RequestType s_requestType ALIGNED;
static bool s_isProbeReplyReceived ALIGNED;

namespace net {
namespace globals {
extern struct IpInfo ipInfo;
extern uint8_t macAddress[ETH_ADDR_LEN];
}  // namespace globals
}  // namespace net

void __attribute__((cold)) arp_init() {
	arp_cache_init();

	s_requestType = net::arp::RequestType::REQUEST;

	// ARP Request template
	// Ethernet header
	memcpy(s_arp_request.ether.src, net::globals::macAddress, ETH_ADDR_LEN);
	memset(s_arp_request.ether.dst, 0xFF , ETH_ADDR_LEN);
	s_arp_request.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

	// ARP Header
	s_arp_request.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_request.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_request.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_request.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_request.arp.opcode = __builtin_bswap16(ARP_OPCODE_RQST);

	memcpy(s_arp_request.arp.sender_mac, net::globals::macAddress, ETH_ADDR_LEN);
	net::memcpy_ip(s_arp_request.arp.sender_ip, net::globals::ipInfo.ip.addr);
	memset(s_arp_request.arp.target_mac, 0x00, ETH_ADDR_LEN);

	// ARP Reply Template
	// Ethernet header
	memcpy(s_arp_reply.ether.src, net::globals::macAddress, ETH_ADDR_LEN);
	s_arp_reply.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

	// ARP Header
	s_arp_reply.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_reply.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_reply.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_reply.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_reply.arp.opcode = __builtin_bswap16(ARP_OPCODE_REPLY);

	memcpy(s_arp_reply.arp.sender_mac, net::globals::macAddress, ETH_ADDR_LEN);
}

bool arp_do_probe() {
#ifndef NDEBUG
	static constexpr uint32_t TIMEOUT_MILLIS = 500;
#else
	static constexpr uint32_t TIMEOUT_MILLIS = 10;
#endif

	auto nRetries = 3;

	while (nRetries--) {
		arp_send_probe();

		Hardware::Get()->WatchdogFeed();

		const auto nMillis = Hardware::Get()->Millis();

		do {
			net_handle();
			if (s_isProbeReplyReceived) {
				return true;
			}
		} while ((Hardware::Get()->Millis() - nMillis) < TIMEOUT_MILLIS);
	}

	return false;
}

void arp_send_request(uint32_t nIp) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR, IP2STR(nIp));

	s_requestType = net::arp::RequestType::REQUEST;

	net::memcpy_ip(s_arp_request.arp.target_ip, nIp);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_request), sizeof(struct t_arp));

	DEBUG_EXIT
}

/*
 *  The Sender IP is set to all zeros,
 *  which means it cannot map to the Sender MAC address.
 *  The Target MAC address is all zeros,
 *  which means it cannot map to the Target IP address.
 */
void arp_send_probe() {
	DEBUG_ENTRY

	s_requestType = net::arp::RequestType::PROBE;
	s_isProbeReplyReceived = false;

	memset(s_arp_request.arp.sender_ip, 0, IPv4_ADDR_LEN);

	arp_send_request(net::globals::ipInfo.ip.addr);

	net::memcpy_ip(s_arp_request.arp.sender_ip, net::globals::ipInfo.ip.addr);

	DEBUG_EXIT
}

/*
 * The packet structure is identical to the ARP Probe above,
 * with the exception that a complete mapping exists.
 * Both the Sender MAC address and the Sender IP address create a complete ARP mapping,
 * and hosts on the network can use this pair of addresses in their ARP table.
 */
void arp_send_announcement() {
	DEBUG_ENTRY

	s_requestType = net::arp::RequestType::ANNNOUNCEMENT;

	arp_send_request(net::globals::ipInfo.ip.addr);

	DEBUG_EXIT
}

void arp_handle_request(struct t_arp *p_arp) {
	DEBUG_ENTRY

	const auto nIpTarget = net::memcpy_ip(p_arp->arp.target_ip);

#ifndef NDEBUG
	const auto nIpSender = net::memcpy_ip(p_arp->arp.sender_ip);
	DEBUG_PRINTF("Sender " IPSTR " Target " IPSTR, IP2STR(nIpSender), IP2STR(nIpTarget));
#endif

	if (!((nIpTarget == net::globals::ipInfo.ip.addr) || (nIpTarget == net::globals::ipInfo.secondary_ip.addr) || (nIpTarget == net::globals::ipInfo.broadcast_ip.addr))) {
		DEBUG_PUTS("No for me.");
		DEBUG_EXIT
		return;
	}

	// Ethernet header
	memcpy(s_arp_reply.ether.dst, p_arp->ether.src, ETH_ADDR_LEN);
	// ARP Header
	memcpy(s_arp_reply.arp.target_mac, p_arp->arp.sender_mac, ETH_ADDR_LEN);
	memcpy(s_arp_reply.arp.target_ip, p_arp->arp.sender_ip, IPv4_ADDR_LEN);
	net::memcpy_ip(s_arp_reply.arp.sender_ip, nIpTarget);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_reply), sizeof(struct t_arp));

	DEBUG_EXIT
}

void arp_handle_reply(struct t_arp *p_arp) {
	DEBUG_ENTRY

	switch (s_requestType) {
	case net::arp::RequestType::REQUEST: {
		arp_cache_update(p_arp->arp.sender_mac, net::memcpy_ip(p_arp->arp.sender_ip));
	}
		break;
	case net::arp::RequestType::PROBE:
		s_isProbeReplyReceived = true;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	DEBUG_EXIT
}

__attribute__((hot)) void arp_handle(struct t_arp *pArp) {
	DEBUG_ENTRY

	switch (pArp->arp.opcode) {
		case __builtin_bswap16(ARP_OPCODE_RQST):
			arp_handle_request(pArp);
			break;
		case __builtin_bswap16(ARP_OPCODE_REPLY):
			arp_handle_reply(pArp);
			break;
		default:
			DEBUG_PRINTF("opcode %04x not handled", __builtin_bswap16(pArp->arp.opcode));
			break;
	}

	DEBUG_EXIT
}
