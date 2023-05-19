/**
 * @file net.cpp
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

#include "net.h"
#include "net_private.h"
#include "net_packets.h"
#include "net_debug.h"

#include "../../config/net_config.h"

namespace net {
namespace globals {
struct IpInfo ipInfo  ALIGNED;
uint32_t nBroadcastIp;
uint32_t nBroadcastMask;
uint32_t nOnNetworkMask;
uint8_t macAddress[ETH_ADDR_LEN]  ALIGNED;
}  // namespace globals
}  // namespace net

static uint8_t *s_p;
static bool s_isDhcp = false;

void __attribute__((cold)) net_init(const uint8_t *const pMacAddress, struct IpInfo *pIpInfo, const char *pHostname, bool *bUseDhcp, bool *isZeroconfUsed) {
	DEBUG_ENTRY

	memcpy(net::globals::macAddress, pMacAddress, ETH_ADDR_LEN);

	const auto *pSrc = reinterpret_cast<const uint8_t *>(pIpInfo);
	auto *pDst = reinterpret_cast<uint8_t *>(&net::globals::ipInfo);

	memcpy(pDst, pSrc, sizeof(struct IpInfo));

	ip_init();

	*isZeroconfUsed = false;

	if (*bUseDhcp) {
		if (dhcp_client(pHostname) < 0) {
			*bUseDhcp = false;
			DEBUG_PUTS("DHCP Client failed");
			*isZeroconfUsed = rfc3927();
		}
	}

	net::globals::nBroadcastIp = net::globals::ipInfo.ip.addr | ~net::globals::ipInfo.netmask.addr;
	net::globals::nBroadcastMask = ~(net::globals::ipInfo.netmask.addr);
	net::globals::nOnNetworkMask = net::globals::ipInfo.ip.addr & net::globals::ipInfo.netmask.addr;

	arp_init();
	ip_set_ip();

	pSrc = reinterpret_cast<const uint8_t *>(&net::globals::ipInfo);
	pDst = reinterpret_cast<uint8_t *>(pIpInfo);

	memcpy(pDst, pSrc, sizeof(struct IpInfo));

	s_isDhcp = *bUseDhcp;

	if (!arp_do_probe()) {
		DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(net::globals::ipInfo.ip.addr), MAC2STR(net::globals::macAddress));
		arp_send_announcement();
	} else {
		console_error("IP Conflict!\n");
	}

	DEBUG_EXIT
}

void __attribute__((cold)) net_shutdown() {
	ip_shutdown();

	if (s_isDhcp) {
		dhcp_client_release();
	}
}

void net_set_ip(uint32_t nIp) {
	net::globals::ipInfo.ip.addr = nIp;

	net::globals::nBroadcastIp = net::globals::ipInfo.ip.addr | ~net::globals::ipInfo.netmask.addr;
	net::globals::nBroadcastMask = ~(net::globals::ipInfo.netmask.addr);
	net::globals::nOnNetworkMask = net::globals::ipInfo.ip.addr & net::globals::ipInfo.netmask.addr;

	arp_init();
	ip_set_ip();

	if (!arp_do_probe()) {
		DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(net::globals::ipInfo.ip.addr), MAC2STR(net::globals::macAddress));
		arp_send_announcement();
	} else {
		console_error("IP Conflict!\n");
	}
}

void net_set_netmask(uint32_t nNetmask) {
	net::globals::ipInfo.netmask.addr = nNetmask;

	net::globals::nBroadcastIp = net::globals::ipInfo.ip.addr | ~net::globals::ipInfo.netmask.addr;
	net::globals::nBroadcastMask = ~(net::globals::ipInfo.netmask.addr);
	net::globals::nOnNetworkMask = net::globals::ipInfo.ip.addr & net::globals::ipInfo.netmask.addr;
}

void net_set_gw(uint32_t nGw) {
	net::globals::ipInfo.gw.addr = nGw;

	ip_set_ip();
}

bool net_set_dhcp(struct IpInfo *p_ip_info, const char *const pHostname, bool *isZeroconfUsed) {
	bool isDhcp = false;
	*isZeroconfUsed = false;

	if (dhcp_client(pHostname) < 0) {
		DEBUG_PUTS("DHCP Client failed");
		*isZeroconfUsed = rfc3927();
	} else {
		isDhcp = true;
	}

	arp_init();
	ip_set_ip();

	const auto *pSrc = reinterpret_cast<const uint8_t *>(&net::globals::ipInfo);
	auto *pDst = reinterpret_cast<uint8_t *>(p_ip_info);

	memcpy(pDst, pSrc, sizeof(struct IpInfo));

	s_isDhcp = isDhcp;

	if (!arp_do_probe()) {
		DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(net::globals::ipInfo.ip.addr), MAC2STR(net::globals::macAddress));
		arp_send_announcement();
	} else {
		console_error("IP Conflict!\n");
	}

	return isDhcp;
}

void net_dhcp_release() {
	dhcp_client_release();
	s_isDhcp = false;
}

bool net_set_zeroconf(struct IpInfo *p_ip_info) {
	const auto b = rfc3927();

	if (b) {
		arp_init();
		ip_set_ip();

		const auto *pSrc = reinterpret_cast<const uint8_t *>(&net::globals::ipInfo);
		auto *pDst = reinterpret_cast<uint8_t *>(p_ip_info);

		memcpy(pDst, pSrc, sizeof(struct IpInfo));

		s_isDhcp = false;

		DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(net::globals::ipInfo.ip.addr), MAC2STR(net::globals::macAddress));
		arp_send_announcement();

		return true;
	}

	DEBUG_PUTS("Zeroconf failed");
	return false;
}

__attribute__((hot)) void net_handle() {
	const auto nLength = emac_eth_recv(&s_p);

	if (__builtin_expect((nLength > 0), 0)) {
		const auto *const eth = reinterpret_cast<struct ether_header *>(s_p);

		if (eth->type == __builtin_bswap16(ETHER_TYPE_IPv4)) {
			ip_handle(reinterpret_cast<struct t_ip4 *>(s_p));
		} else if (eth->type == __builtin_bswap16(ETHER_TYPE_ARP)) {
			arp_handle(reinterpret_cast<struct t_arp *>(s_p));
		} else {
			DEBUG_PRINTF("type %04x is not implemented", __builtin_bswap16(eth->type));
		}

		emac_free_pkt();
	}

	net_timers_run();
}
