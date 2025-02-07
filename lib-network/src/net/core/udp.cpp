/**
 * @file udp.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "net_config.h"
#include "net/protocol/udp.h"
#include "net/udp.h"
#include "net_private.h"
#include "net_memcpy.h"

#include "debug.h"

namespace net {
namespace globals {
extern uint32_t nBroadcastMask;
}  // namespace globals

struct PortInfo {
	UdpCallbackFunctionPtr callback;
	uint16_t nPort;
};

struct Data {
	uint32_t nFromIp;
	uint32_t nSize;
	uint8_t data[UDP_DATA_SIZE];
	uint16_t nFromPort;
};

struct Port {
	PortInfo info;
	Data data ALIGNED;
} ALIGNED;

static Port s_Ports[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] SECTION_NETWORK ALIGNED;

void __attribute__((cold)) udp_init() {
	// Multicast fixed part
	s_multicast_mac[0] = 0x01;
	s_multicast_mac[1] = 0x00;
	s_multicast_mac[2] = 0x5E;
}

void __attribute__((cold)) udp_shutdown() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

__attribute__((hot)) void udp_input(const struct t_udp *pUdp) {
	const auto nDestinationPort = __builtin_bswap16(pUdp->udp.destination_port);

	for (uint32_t nPortIndex = 0; nPortIndex < UDP_MAX_PORTS_ALLOWED; nPortIndex++) {
		const auto& portInfo = s_Ports[nPortIndex].info;
		auto& data = s_Ports[nPortIndex].data;

		if (portInfo.nPort == nDestinationPort) {
			if (__builtin_expect ((data.nSize != 0), 0)) {
				DEBUG_PRINTF("%d[%x]", nDestinationPort, nDestinationPort);
			}

			const auto nDataLength = static_cast<uint32_t>(__builtin_bswap16(pUdp->udp.len) - UDP_HEADER_SIZE);
			const auto i = std::min(static_cast<uint32_t>(UDP_DATA_SIZE), nDataLength);

			net::memcpy(data.data, pUdp->udp.data, i);

			data.nFromIp = net::memcpy_ip(pUdp->ip4.src);
			data.nFromPort = __builtin_bswap16(pUdp->udp.source_port);
			data.nSize = i;

			emac_free_pkt();

			if (portInfo.callback != nullptr) {
				portInfo.callback(data.data, nDataLength, data.nFromIp, data.nFromPort);
			}

			return;
		}
	}

	emac_free_pkt();

	DEBUG_PRINTF(IPSTR ":%d[%x] " MACSTR, pUdp->ip4.src[0],pUdp->ip4.src[1],pUdp->ip4.src[2],pUdp->ip4.src[3], nDestinationPort, nDestinationPort, MAC2STR(pUdp->ether.dst));
}

template<net::arp::EthSend S>
static void udp_send_implementation(int nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);
	assert(s_Ports[nIndex].info.nPort != 0);

	auto *pOutBuffer = reinterpret_cast<t_udp *>(emac_eth_send_get_dma_buffer());

	// Ethernet
	std::memcpy(pOutBuffer->ether.src, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	pOutBuffer->ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);

	//IPv4
	pOutBuffer->ip4.ver_ihl = 0x45;
	pOutBuffer->ip4.tos = 0;
	pOutBuffer->ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	pOutBuffer->ip4.ttl = 64;
	pOutBuffer->ip4.proto = IPv4_PROTO_UDP;
	pOutBuffer->ip4.id = ++s_id;
	pOutBuffer->ip4.len = __builtin_bswap16(static_cast<uint16_t>(nSize + IPv4_UDP_HEADERS_SIZE));
	pOutBuffer->ip4.chksum = 0;
	net::memcpy_ip(pOutBuffer->ip4.src, net::globals::netif_default.ip.addr);

	//UDP
	pOutBuffer->udp.source_port = __builtin_bswap16(s_Ports[nIndex].info.nPort);
	pOutBuffer->udp.destination_port = __builtin_bswap16(nRemotePort);
	pOutBuffer->udp.len = __builtin_bswap16(static_cast<uint16_t>(nSize + UDP_HEADER_SIZE));
	pOutBuffer->udp.checksum = 0;

	nSize = std::min(static_cast<uint32_t>(UDP_DATA_SIZE), nSize);

	net::memcpy(pOutBuffer->udp.data, pData, nSize);

	if (nRemoteIp == net::IPADDR_BROADCAST) {
		net::memset<0xFF, ETH_ADDR_LEN>(pOutBuffer->ether.dst);
		net::memset<0xFF, IPv4_ADDR_LEN>(pOutBuffer->ip4.dst);
	} else if ((nRemoteIp & net::globals::nBroadcastMask) == net::globals::nBroadcastMask) {
		net::memset<0xFF, ETH_ADDR_LEN>(pOutBuffer->ether.dst);
		net::memcpy_ip(pOutBuffer->ip4.dst, nRemoteIp);
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

			std::memcpy(pOutBuffer->ether.dst, s_multicast_mac, ETH_ADDR_LEN);
			net::memcpy_ip(pOutBuffer->ip4.dst, nRemoteIp);
		} else {
			if constexpr (S == net::arp::EthSend::IS_NORMAL) {
				net::arp_send(pOutBuffer, nSize + UDP_PACKET_HEADERS_SIZE, nRemoteIp);
			}
#if defined CONFIG_NET_ENABLE_PTP
			else if constexpr (S == net::arp::EthSend::IS_TIMESTAMP) {
				net::arp_send_timestamp(pOutBuffer, nSize + UDP_PACKET_HEADERS_SIZE, nRemoteIp);
			}
#endif
			return;
		}
	}

#if !defined (CHECKSUM_BY_HARDWARE)
	pOutBuffer->ip4.chksum = net_chksum(reinterpret_cast<void *>(&pOutBuffer->ip4), sizeof(pOutBuffer->ip4));
#endif

	if constexpr (S == net::arp::EthSend::IS_NORMAL) {
		emac_eth_send(nSize + UDP_PACKET_HEADERS_SIZE);
	}
#if defined CONFIG_NET_ENABLE_PTP
	else if constexpr (S == net::arp::EthSend::IS_TIMESTAMP) {
		emac_eth_send_timestamp(nSize);
	}
#endif
	return;
}

// -->

int32_t udp_begin(uint16_t nLocalPort, UdpCallbackFunctionPtr callback) {
	DEBUG_PRINTF("nLocalPort=%u", nLocalPort);

	for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++) {
		auto& portInfo = s_Ports[i].info;

		if (portInfo.nPort == nLocalPort) {
			return i;
		}

		if (portInfo.nPort == 0) {
			portInfo.callback = callback;
			portInfo.nPort = nLocalPort;

			DEBUG_PRINTF("i=%d, local_port=%d[%x], callback=%p", i, nLocalPort, nLocalPort, callback);
			return i;
		}
	}

#ifndef NDEBUG
	console_error("udp_begin\n");
#endif
	return -1;
}

int32_t udp_end(uint16_t nLocalPort) {
	DEBUG_PRINTF("nLocalPort=%u[%x]", nLocalPort, nLocalPort);

	for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++) {
		auto& portInfo = s_Ports[i].info;

		if (portInfo.nPort == nLocalPort) {
			portInfo.callback = nullptr;
			portInfo.nPort = 0;

			auto& data = s_Ports[i].data;
			data.nSize = 0;
			return 0;
		}
	}

	console_error("unbind\n");
	return -1;
}

uint32_t udp_recv1(const int32_t nIndex, uint8_t *pData, uint32_t nSize, uint32_t *pFromIp, uint16_t *FromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	auto& data = s_Ports[nIndex].data;

	if (__builtin_expect((data.nSize == 0), 1)) {
		return 0;
	}

	const auto i = std::min(nSize, data.nSize);

	net::memcpy(pData, data.data, i);

	*pFromIp = data.nFromIp;
	*FromPort = data.nFromPort;

	data.nSize = 0;

	return i;
}

uint32_t udp_recv2(const int32_t nIndex, const uint8_t **pData, uint32_t *pFromIp, uint16_t *pFromPort) {
	assert(nIndex >= 0);
	assert(nIndex < UDP_MAX_PORTS_ALLOWED);

	const auto& portInfo = s_Ports[nIndex].info;

	if (__builtin_expect(portInfo.callback != nullptr, 0)) {
		return 0;
	}

	auto& data = s_Ports[nIndex].data;

	if (__builtin_expect((data.nSize == 0), 1)) {
		return 0;
	}

	*pData = data.data;
	*pFromIp = data.nFromIp;
	*pFromPort = data.nFromPort;

	const auto nSize = data.nSize;

	data.nSize = 0;

	return nSize;
}

void udp_send(const int32_t nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	udp_send_implementation<net::arp::EthSend::IS_NORMAL>(nIndex, pData, nSize, nRemoteIp, nRemotePort);
}

#if defined CONFIG_NET_ENABLE_PTP
void udp_send_timestamp(const int32_t nIndex, const uint8_t *pData, uint32_t nSize, uint32_t nRemoteIp, uint16_t nRemotePort) {
	udp_send_implementation<net::arp::EthSend::IS_TIMESTAMP>(nIndex, pData, nSize, nRemoteIp, nRemotePort);
}
#endif
}  // namespace net
// <---
