/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef EMAC_NETWORK_H_
#define EMAC_NETWORK_H_

#if defined (NO_EMAC) || defined (ESP8266)
# error This file should not be included
#endif

namespace net {
#if defined (CONFIG_NET_ENABLE_PTP)
void ptp_run();
#endif
}  // namespace net

#include <cstdint>
#include <cstring>
#include <cassert>
#include <net/if.h>

#include "networkparams.h"

#include "net.h"
#include "netif.h"
#include "net/net.h"
#include "net/ip4_address.h"
#include "net/netif.h"
#include "net/igmp.h"
#include "net/udp.h"
#include "net/tcp.h"
#include "net/dhcp.h"

#include "emac/net_link_check.h"

namespace net {
void ethernet_input(const uint8_t *pBuffer, const uint32_t nLength);
}  // namespace net

void network_init();
uint32_t emac_eth_recv(uint8_t **ppPacket);

namespace global::network {
extern net::Link linkState;
}  // namespace global::network

class Network {
public:
	Network() {
		DEBUG_ENTRY
		assert(s_pThis == nullptr);
		s_pThis = this;

		network_init();

		DEBUG_EXIT
	}
	~Network() = default;

	void MacAddressCopyTo(uint8_t *pMacAddress) {
		memcpy(pMacAddress, net::netif_hwaddr(), NETIF_MAX_HWADDR_LEN);
	}

	uint32_t GetSecondaryIp() {
		return net::netif_secondary_ipaddr();
	}

	void SetIp(const uint32_t nIp) {
		net::net_set_primary_ip(nIp);
	}
	uint32_t GetIp() {
		return net::net_get_primary_ip();
	}

	void SetNetmask(const uint32_t nNetmask) {
		net::net_set_netmask(nNetmask);
	}
	uint32_t GetNetmask() {
		return net::net_get_netmask();
	}

	void SetGatewayIp(const uint32_t nGatewayIp) {
		net::net_set_gateway_ip(nGatewayIp);
	}
	uint32_t GetGatewayIp()  {
		return net::net_get_gateway_ip();
	}

	uint32_t GetBroadcastIp()  {
		return net::netif_broadcast_ipaddr();
	}

	/*
	 * DHCP
	 */

	bool IsDhcpCapable() const {
		return net::net_is_dhcp_capable();
	}

	void EnableDhcp() {
		net::net_enable_dhcp();
	}

	bool IsDhcpUsed() {
		return net::netif_dhcp();
	}

	bool IsDhcpKnown() const {
		return net::net_is_dhcp_known();
	}

	/*
	 * Zeroconf / autoip
	 */

	void SetZeroconf() {
		net::net_set_zeroconf();
	}
	bool IsZeroconfUsed() const {
		return net::net_is_zeroconf_used();
	}
	bool IsZeroconfCapable() const {
		return net::net_is_zeroconf_capable();
	}

	/*
	 * Host name
	 */

	void SetHostName(const char *pHostName) {
		net::netif_set_hostname(pHostName);
	}
	const char *GetHostName() const {
		return net::netif_get_hostname();
	}

	/*
	 * Domain name
	 */

	void SetDomainName(const char *pDomainName) {
		net::netif_set_domainname(pDomainName);
	}
	const char *GetDomainName() const {
		return net::netif_get_domainname();
	}

	/*
	 * Name servers
	 */

	uint32_t GetNameServer(const uint32_t nIndex) {
		return net::netif_get_nameserver(nIndex);
	}

	uint32_t GetNameServers() const {
		return net::netif_get_nameservers();
	}

	const char *GetIfName() {
		return net::netif_get_ifname();
	}

	uint32_t GetIfIndex() const {
		return net::netif_get_ifindex();
	}

	/*
	 * UDP/IP
	 */

	int32_t Begin(uint16_t nPort, net::UdpCallbackFunctionPtr callback = nullptr) {
		return net::udp_begin(nPort, callback);
	}

	int32_t End(uint16_t nPort) {
		return net::udp_end(nPort);
	}

	uint32_t RecvFrom(int32_t nHandle, void *pBuffer, uint32_t nLength, uint32_t *from_ip, uint16_t *from_port) {
		return net::udp_recv1(nHandle, reinterpret_cast<uint8_t *>(pBuffer), nLength, from_ip, from_port);
	}

	uint32_t RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort) {
		return net::udp_recv2(nHandle, reinterpret_cast<const uint8_t **>(ppBuffer), pFromIp, pFromPort);
	}

	void SendTo(int32_t nHandle, const void *pBuffer, uint32_t nLength, uint32_t to_ip, uint16_t remote_port) {
		if (__builtin_expect((GetIp() != 0), 1)) { //FIXME
			net::udp_send(nHandle, reinterpret_cast<const uint8_t *>(pBuffer), nLength, to_ip, remote_port);
		}
	}

	void SendToTimestamp(int32_t nHandle, const void *pBuffer, uint32_t nLength, uint32_t to_ip, uint16_t remote_port) {
		net::udp_send_timestamp(nHandle, reinterpret_cast<const uint8_t *>(pBuffer), nLength, to_ip, remote_port);
	}

	/*
	 * IGMP
	 */

	void JoinGroup([[maybe_unused]] int32_t nHandle, uint32_t nIp) {
		net::igmp_join(nIp);
	}

	void LeaveGroup([[maybe_unused]] int32_t nHandle, uint32_t nIp) {
		net::igmp_leave(nIp);
	}

	uint32_t GetNetmaskCIDR() {
		return static_cast<uint32_t>(__builtin_popcount(GetNetmask()));
	}

	char GetAddressingMode() {
		return net::net_get_addressing_mode();
	}

	bool IsValidIp(const uint32_t nIp) {
		return net::net_is_valid_ip(nIp);
	}

	void Run() {
		uint8_t *pEthernetBuffer;
		const auto nLength = emac_eth_recv(&pEthernetBuffer);

		if (__builtin_expect((nLength > 0), 0)) {
			net::ethernet_input(pEthernetBuffer, nLength);
		}
#if defined (CONFIG_NET_ENABLE_PTP)
		net::ptp_run();
#endif
#if defined (ENET_LINK_CHECK_USE_PIN_POLL)
		net::link_pin_poll();
#elif defined (ENET_LINK_CHECK_REG_POLL)
		const net::Link link_state = net::link_status_read();
		if (link_state != global::network::linkState) {
			global::network::linkState = link_state;
			net::link_handle_change(link_state);
		}
#endif
	}

	static Network *Get() {
		return s_pThis;
	}

private:
	static inline Network *s_pThis;
};

#endif /* EMAC_NETWORK_H_ */
