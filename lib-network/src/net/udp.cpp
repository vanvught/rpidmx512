/**
 * @file udp.cpp
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

#if defined (DEBUG_NET_UDP)
# undef NDEBUG
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "../../config/net_config.h"

#include "net/protocol/udp.h"

#include "net.h"
#include "net_private.h"
#include "net_memcpy.h"

#include "debug.h"

namespace net {
namespace globals {
extern uint32_t nBroadcastMask;
}  // namespace globals

struct data_entry {
	uint32_t from_ip;
	uint32_t size;
	uint16_t from_port;
	uint8_t data[UDP_DATA_SIZE];
} ALIGNED;

static uint16_t s_Port[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct data_entry s_data[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct t_udp s_send_packet SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] SECTION_NETWORK ALIGNED;

void udp_set_ip() {
	net::memcpy_ip(s_send_packet.ip4.src, net::globals::netif_default.ip.addr);
}

void __attribute__((cold)) udp_init() {
	// Multicast fixed part
	s_multicast_mac[0] = 0x01;
	s_multicast_mac[1] = 0x00;
	s_multicast_mac[2] = 0x5E;
	// Ethernet
	std::memcpy(s_send_packet.ether.src, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	s_send_packet.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
	// IPv4
	s_send_packet.ip4.ver_ihl = 0x45;
	s_send_packet.ip4.tos = 0;
	s_send_packet.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_send_packet.ip4.ttl = 64;
	s_send_packet.ip4.proto = IPv4_PROTO_UDP;
	udp_set_ip();
	// UDP
	s_send_packet.udp.checksum = 0;
}

void __attribute__((cold)) udp_shutdown() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

__attribute__((hot)) void udp_handle(struct t_udp *pUdp) {
	const auto nDestinationPort = __builtin_bswap16(pUdp->udp.destination_port);

	for (uint32_t nPortIndex = 0; nPortIndex < UDP_MAX_PORTS_ALLOWED; nPortIndex++) {
		if (s_Port[nPortIndex] == nDestinationPort) {
			if (__builtin_expect ((s_data[nPortIndex].size != 0), 0)) {
				DEBUG_PRINTF(IPSTR ":%d[%x]", pUdp->ip4.src[0],pUdp->ip4.src[1],pUdp->ip4.src[2],pUdp->ip4.src[3], nDestinationPort, nDestinationPort);
			}

			auto *p_queue_entry = &s_data[nPortIndex];
			const auto nDataLength = static_cast<uint16_t>(__builtin_bswap16(pUdp->udp.len) - UDP_HEADER_SIZE);
			const auto i = std::min(static_cast<uint16_t>(UDP_DATA_SIZE), nDataLength);

			net::memcpy(p_queue_entry->data, pUdp->udp.data, i);

			p_queue_entry->from_ip = net::memcpy_ip(pUdp->ip4.src);
			p_queue_entry->from_port = __builtin_bswap16(pUdp->udp.source_port);
			p_queue_entry->size = static_cast<uint16_t>(i);

			return;
		}
	}

	DEBUG_PRINTF(IPSTR ":%d[%x] " MACSTR, pUdp->ip4.src[0],pUdp->ip4.src[1],pUdp->ip4.src[2],pUdp->ip4.src[3], nDestinationPort, nDestinationPort, MAC2STR(pUdp->ether.dst));
}

template<net::arp::EthSend S>
static void udp_send_implementation(int nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);
	assert(s_Port[nIndex] != 0);

	//IPv4
	s_send_packet.ip4.id = s_id++;
	s_send_packet.ip4.len = __builtin_bswap16(static_cast<uint16_t>(nSize + IPv4_UDP_HEADERS_SIZE));
	s_send_packet.ip4.chksum = 0;

	//UDP
	s_send_packet.udp.source_port = __builtin_bswap16( s_Port[nIndex]);
	s_send_packet.udp.destination_port = __builtin_bswap16(nRemotePort);
	s_send_packet.udp.len = __builtin_bswap16(static_cast<uint16_t>(nSize + UDP_HEADER_SIZE));

	nSize = std::min(static_cast<uint32_t>(UDP_DATA_SIZE), nSize);

	net::memcpy(s_send_packet.udp.data, pData, nSize);

	if (nRemoteIp == network::IP4_BROADCAST) {
		memset(s_send_packet.ether.dst, 0xFF, ETH_ADDR_LEN);
		memset(s_send_packet.ip4.dst, 0xFF, IPv4_ADDR_LEN);
	} else if ((nRemoteIp & net::globals::nBroadcastMask) == net::globals::nBroadcastMask) {
		memset(s_send_packet.ether.dst, 0xFF, ETH_ADDR_LEN);
		net::memcpy_ip(s_send_packet.ip4.dst, nRemoteIp);
	} else {
		if ((nRemoteIp & 0xF0) == 0xE0) { // Multicast, we know the MAC Address
			typedef union pcast32 {
				uint32_t u32;
				uint8_t u8[4];
			} _pcast32;
			_pcast32 multicast_ip;

			multicast_ip.u32 = nRemoteIp;
			s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
			s_multicast_mac[4] = multicast_ip.u8[2];
			s_multicast_mac[5] = multicast_ip.u8[3];

			std::memcpy(s_send_packet.ether.dst, s_multicast_mac, ETH_ADDR_LEN);
			net::memcpy_ip(s_send_packet.ip4.dst, nRemoteIp);
		} else {
			if (S == net::arp::EthSend::IS_NORMAL) {
				net::arp_send(&s_send_packet, nSize + UDP_PACKET_HEADERS_SIZE, nRemoteIp);
			}
#if defined CONFIG_ENET_ENABLE_PTP
			else if (S == net::arp::EthSend::IS_TIMESTAMP) {
				net::arp_send_timestamp(&s_send_packet, nSize + UDP_PACKET_HEADERS_SIZE, nRemoteIp);
			}
#endif
			return;
		}
	}

#if !defined (CHECKSUM_BY_HARDWARE)
	s_send_packet.ip4.chksum = net_chksum(reinterpret_cast<void *>(&s_send_packet.ip4), sizeof(s_send_packet.ip4));
#endif

	if (S == net::arp::EthSend::IS_NORMAL) {
		emac_eth_send(reinterpret_cast<void *>(&s_send_packet), nSize + UDP_PACKET_HEADERS_SIZE);
	}
#if defined CONFIG_ENET_ENABLE_PTP
	else if (S == net::arp::EthSend::IS_TIMESTAMP) {
		emac_eth_send_timestamp(reinterpret_cast<void *>(&s_send_packet), nSize);
	}
#endif
	return;
}

// -->

int udp_begin(uint16_t nLocalPort) {
	DEBUG_PRINTF("nLocalPort=%u", nLocalPort);

	for (int i = 0; i < UDP_MAX_PORTS_ALLOWED; i++) {
		if (s_Port[i] == nLocalPort) {
			return i;
		}

		if (s_Port[i] == 0) {
			s_Port[i] = nLocalPort;

			DEBUG_PRINTF("i=%d, local_port=%d[%x]", i, nLocalPort, nLocalPort);
			return i;
		}
	}

#ifndef NDEBUG
	console_error("udp_begin\n");
#endif
	return -1;
}

int udp_end(uint16_t nLocalPort) {
	DEBUG_PRINTF("nLocalPort=%u[%x]", nLocalPort, nLocalPort);

	for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++) {
		if (s_Port[i] == nLocalPort) {
			s_Port[i] = 0;
			s_data[i].size = 0;
			return 0;
		}
	}

	console_error("unbind\n");
	return -1;
}

uint32_t udp_recv1(int nIndex, uint8_t *pData, uint32_t nSize, uint32_t *pFromIp, uint16_t *FromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	if (__builtin_expect((s_data[nIndex].size == 0), 1)) {
		return 0;
	}

	auto *p_data = &s_data[nIndex];
	const auto i = std::min(nSize, p_data->size);

	net::memcpy(pData, p_data->data, i);

	*pFromIp = p_data->from_ip;
	*FromPort = p_data->from_port;

	p_data->size = 0;

	return i;
}

uint32_t udp_recv2(int nIndex, const uint8_t **pData, uint32_t *pFromIp, uint16_t *pFromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	auto &p_data = s_data[nIndex];

	if (__builtin_expect((p_data.size == 0), 1)) {
		return 0;
	}

	*pData = p_data.data;
	*pFromIp = p_data.from_ip;
	*pFromPort = p_data.from_port;

	const auto nSize = p_data.size;

	p_data.size = 0;

	return nSize;
}

void udp_send(int nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	udp_send_implementation<net::arp::EthSend::IS_NORMAL>(nIndex, pData, nSize, nRemoteIp, nRemotePort);
}

#if defined CONFIG_ENET_ENABLE_PTP
void udp_send_timestamp(int nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	udp_send_implementation<net::arp::EthSend::IS_TIMESTAMP>(nIndex, pData, nSize, nRemoteIp, nRemotePort);
}
#endif
}  // namespace net
// <---
