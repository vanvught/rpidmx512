/**
 * @file udp.cpp
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
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "net.h"
#include "net_private.h"
#include "net_memcpy.h"

#include "../../config/net_config.h"

struct data_entry {
	uint32_t from_ip;
	uint16_t from_port;
	uint16_t size;
	uint8_t data[UDP_DATA_SIZE];
} ALIGNED;

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static uint16_t s_Port[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct data_entry s_data[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct t_udp s_send_packet SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] SECTION_NETWORK ALIGNED;

namespace net {
namespace globals {
extern struct IpInfo ipInfo;
extern uint32_t nBroadcastMask;
extern uint32_t nOnNetworkMask;
extern uint8_t macAddress[ETH_ADDR_LEN];
}  // namespace globals
}  // namespace net

void udp_set_ip() {
	_pcast32 src;

	src.u32 = net::globals::ipInfo.ip.addr;
	memcpy(s_send_packet.ip4.src, src.u8, IPv4_ADDR_LEN);
}

void __attribute__((cold)) udp_init() {
	// Multicast fixed part
	s_multicast_mac[0] = 0x01;
	s_multicast_mac[1] = 0x00;
	s_multicast_mac[2] = 0x5E;
	// Ethernet
	memcpy(s_send_packet.ether.src, net::globals::macAddress, ETH_ADDR_LEN);
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

			net_memcpy(p_queue_entry->data, pUdp->udp.data, i);

			_pcast32 src;

			memcpy(src.u8, pUdp->ip4.src, IPv4_ADDR_LEN);
			p_queue_entry->from_ip = src.u32;
			p_queue_entry->from_port = __builtin_bswap16(pUdp->udp.source_port);
			p_queue_entry->size = static_cast<uint16_t>(i);

			return;

		}
	}

	DEBUG_PRINTF(IPSTR ":%d[%x]", pUdp->ip4.src[0],pUdp->ip4.src[1],pUdp->ip4.src[2],pUdp->ip4.src[3], nDestinationPort, nDestinationPort);
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

uint16_t udp_recv1(int nIndex, uint8_t *pData, uint16_t nSize, uint32_t *pFromIp, uint16_t *FromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	if (__builtin_expect((s_data[nIndex].size == 0), 1)) {
		return 0;
	}

	auto *p_data = &s_data[nIndex];
	const auto i = std::min(nSize, p_data->size);

	net_memcpy(pData, p_data->data, i);

	*pFromIp = p_data->from_ip;
	*FromPort = p_data->from_port;

	p_data->size = 0;

	return i;
}

uint16_t udp_recv2(int nIndex, const uint8_t **pData, uint32_t *pFromIp, uint16_t *pFromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	if (__builtin_expect((s_data[nIndex].size == 0), 1)) {
		return 0;
	}

	auto *p_data = &s_data[nIndex];

	*pData = p_data->data;
	*pFromIp = p_data->from_ip;
	*pFromPort = p_data->from_port;

	const auto nSize = p_data->size;

	p_data->size = 0;

	return nSize;
}

int udp_send(int nIndex, const uint8_t *pData, uint16_t nSize, uint32_t RemoteIp, uint16_t RemotePort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);
	_pcast32 dst;

	if (__builtin_expect ((s_Port[nIndex] == 0), 0)) {
		DEBUG_PUTS("ports_allowed[idx] == 0");
		return -1;
	}

	if (RemoteIp == IPv4_BROADCAST) {
		memset(s_send_packet.ether.dst, 0xFF, ETH_ADDR_LEN);
		memset(s_send_packet.ip4.dst, 0xFF, IPv4_ADDR_LEN);
	} else if ((RemoteIp & net::globals::nBroadcastMask) == net::globals::nBroadcastMask) {
		memset(s_send_packet.ether.dst, 0xFF, ETH_ADDR_LEN);
		dst.u32 = RemoteIp;
		memcpy(s_send_packet.ip4.dst, dst.u8, IPv4_ADDR_LEN);
	} else {
		if ((RemoteIp & 0xE0) == 0xE0) { // Multicast, we know the MAC Address
			_pcast32 multicast_ip;

			multicast_ip.u32 = RemoteIp;

			s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
			s_multicast_mac[4] = multicast_ip.u8[2];
			s_multicast_mac[5] = multicast_ip.u8[3];

			memcpy(s_send_packet.ether.dst, s_multicast_mac, ETH_ADDR_LEN);

			dst.u32 = RemoteIp;
			memcpy(s_send_packet.ip4.dst, dst.u8, IPv4_ADDR_LEN);
		} else {
			if  (__builtin_expect((net::globals::nOnNetworkMask != (RemoteIp & net::globals::nOnNetworkMask)), 0)) {
				if (net::globals::ipInfo.gw.addr == arp_cache_lookup(net::globals::ipInfo.gw.addr, s_send_packet.ether.dst)) {
					dst.u32 = RemoteIp;
					memcpy(s_send_packet.ip4.dst, dst.u8, IPv4_ADDR_LEN);
				} else {
#ifndef NDEBUG
					console_error("ARP lookup failed -> default gateway :");
					printf(IPSTR " [%d]\n", IP2STR(RemoteIp), s_Port[nIndex]);
#endif
					return -3;
				}
			} else {
				if (RemoteIp == arp_cache_lookup(RemoteIp, s_send_packet.ether.dst)) {
					dst.u32 = RemoteIp;
					memcpy(s_send_packet.ip4.dst, dst.u8, IPv4_ADDR_LEN);
				} else {
#ifndef NDEBUG
					console_error("ARP lookup failed: ");
					printf(IPSTR "\n", IP2STR(RemoteIp));
#endif
					return -2;
				}
			}
		}
	}

	//IPv4
	s_send_packet.ip4.id = s_id;
	s_send_packet.ip4.len = __builtin_bswap16((nSize + IPv4_UDP_HEADERS_SIZE));
	s_send_packet.ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
	s_send_packet.ip4.chksum = net_chksum(reinterpret_cast<void *>(&s_send_packet.ip4), sizeof(s_send_packet.ip4));
#endif
	//UDP
	s_send_packet.udp.source_port = __builtin_bswap16( s_Port[nIndex]);
	s_send_packet.udp.destination_port = __builtin_bswap16(RemotePort);
	s_send_packet.udp.len = __builtin_bswap16((nSize + UDP_HEADER_SIZE));

	net_memcpy(s_send_packet.udp.data, pData, std::min(static_cast<uint16_t>(UDP_DATA_SIZE), nSize));

	emac_eth_send(reinterpret_cast<void *>(&s_send_packet), nSize + UDP_PACKET_HEADERS_SIZE);

	s_id++;

	return 0;
}

// <---
