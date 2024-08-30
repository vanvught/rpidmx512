/**
 * network.cpp
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

#ifdef DEBUG_NETWORK
# undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <time.h>
#include <cassert>

#include "network.h"
#include "networkparams.h"
#include "networkstore.h"

#include "hardware.h"

#include "emac/emac.h"
#include "emac/phy.h"
#include "emac/mmi.h"
#include "emac/net_link_check.h"

#include "netif.h"
#include "net/autoip.h"
#include "net/dhcp.h"

#include "../../config/net_config.h"

#include "debug.h"

namespace network {
void __attribute__((weak)) mdns_announcement() {}
}  // namespace network

static constexpr char TO_HEX(const char i) {
	return static_cast<char>(((i) < 10) ? '0' + i : 'A' + (i - 10));
}

#if !defined PHY_ADDRESS
# define PHY_ADDRESS	1
#endif

static void netif_ext_callback(const uint16_t reason, [[maybe_unused]] const net::netif_ext_callback_args_t *args) {
	DEBUG_ENTRY

	if ((reason & net::NetifReason::NSC_IPV4_ADDRESS_CHANGED) == net::NetifReason::NSC_IPV4_ADDRESS_CHANGED) {
		net::display_ip();
		network::mdns_announcement();

		printf("ip: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_address.addr), IP2STR(net::netif_ipaddr()));
	}

	if ((reason & net::NetifReason::NSC_IPV4_NETMASK_CHANGED) == net::NetifReason::NSC_IPV4_NETMASK_CHANGED) {
		net::display_netmask();

		printf("netmask: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_netmask.addr), IP2STR(net::netif_netmask()));
	}

	if ((reason & net::NetifReason::NSC_IPV4_GATEWAY_CHANGED) == net::NetifReason::NSC_IPV4_GATEWAY_CHANGED) {
		net::display_gateway();

		printf("gw: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_gw.addr), IP2STR(net::netif_gw()));
	}

	if ((reason & net::NetifReason::NSC_LINK_CHANGED) == net::NetifReason::NSC_LINK_CHANGED) {
		if (args->link_changed.state == 0) {	// Link down
			net::net_link_down();

			DEBUG_EXIT
			return;
		}
	}

	DEBUG_EXIT
}

Network *Network::s_pThis;

Network::Network() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	strcpy(m_aIfName, "eth0");
	m_aDomainName[0] = '\0';
	memset(&m_nNameservers, 0, sizeof(m_nNameservers));

	net::display_emac_config();

	emac_config();

	net::display_emac_start();

	emac_start(net::globals::netif_default.hwaddr, s_lastState);
	printf(MACSTR "\n", MAC2STR(net::globals::netif_default.hwaddr));

	net::phy_customized_timing();
	net::phy_customized_led();

	net::netif_init();
	net::netif_add_ext_callback(netif_ext_callback);

	NetworkParams params;
	params.Load();

	const auto *p = params.GetHostName();
	assert(p != nullptr);

	if (*p == '\0') {
		uint32_t k = 0;

		for (uint32_t i = 0; (i < (sizeof(HOST_NAME_PREFIX) - 1)) && (i < network::HOSTNAME_SIZE - 7); i++) {
			m_aHostName[k++] = HOST_NAME_PREFIX[i];
		}

		auto hwaddr = net::globals::netif_default.hwaddr;

		m_aHostName[k++] = TO_HEX(hwaddr[3] >> 4);
		m_aHostName[k++] = TO_HEX(hwaddr[3] & 0x0F);
		m_aHostName[k++] = TO_HEX(hwaddr[4] >> 4);
		m_aHostName[k++] = TO_HEX(hwaddr[4] & 0x0F);
		m_aHostName[k++] = TO_HEX(hwaddr[5] >> 4);
		m_aHostName[k++] = TO_HEX(hwaddr[5] & 0x0F);
		m_aHostName[k] = '\0';
	} else {
		strncpy(m_aHostName, p, sizeof(m_aHostName) - 1);
		m_aHostName[sizeof(m_aHostName) - 1] = '\0';
	}

	net::netif_set_hostname(m_aHostName);

	net::ip4_addr_t ipaddr;
	net::ip4_addr_t netmask;
	net::ip4_addr_t gw;

	ipaddr.addr = params.GetIpAddress();
	netmask.addr = params.GetNetMask();
	gw.addr = params.GetDefaultGateway();

	bool isDhcpUsed = params.isDhcpUsed();

	net::display_emac_status(net::Link::STATE_UP == s_lastState);
	net::net_init(s_lastState, ipaddr, netmask, gw, isDhcpUsed);

#if defined (ENET_LINK_CHECK_USE_INT)
	net::link_interrupt_init();
#elif defined (ENET_LINK_CHECK_USE_PIN_POLL)
	net::link_pin_poll_init();
#elif defined (ENET_LINK_CHECK_REG_POLL)
	net::link_status_read();
#endif
	DEBUG_EXIT
}

void Network::SetIp(uint32_t nIp) {
	DEBUG_ENTRY

	if (nIp == net::netif_ipaddr()) {
		DEBUG_EXIT
		return;
	}

	net::ip4_addr_t ipaddr;
	ipaddr.addr = nIp;
	net_set_primary_ip(ipaddr);

	NetworkStore::SaveIp(nIp);
	NetworkStore::SaveDhcp(false);

	DEBUG_EXIT
}

void Network::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (nNetmask == net::netif_netmask()) {
		DEBUG_EXIT
		return;
	}

	net::ip4_addr_t netmask;
	netmask.addr = nNetmask;

	net::netif_set_netmask(netmask);

	NetworkStore::SaveNetMask(nNetmask);

	DEBUG_EXIT
}

void Network::SetGatewayIp(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (nGatewayIp == net::netif_gw()) {
		DEBUG_EXIT
		return;
	}

	net::ip4_addr_t gw;
	gw.addr = nGatewayIp;

	net::netif_set_gw(gw);

	NetworkStore::SaveGatewayIp(nGatewayIp);

	DEBUG_EXIT
}

void Network::SetHostName(const char *pHostName) {
	DEBUG_ENTRY

	strncpy(m_aHostName, pHostName, network::HOSTNAME_SIZE - 1);
	m_aHostName[network::HOSTNAME_SIZE - 1] = '\0';

	NetworkStore::SaveHostName(m_aHostName, static_cast<uint32_t>(strlen(m_aHostName)));

	network::mdns_announcement();
	net::display_hostname();

	DEBUG_EXIT
}

void Network::SetZeroconf() {
	DEBUG_ENTRY

	net::autoip_start();

	NetworkStore::SaveDhcp(false);

	DEBUG_EXIT
}

void Network::EnableDhcp() {
	DEBUG_ENTRY

	net::dhcp_start();

	NetworkStore::SaveDhcp(true);

	DEBUG_EXIT
}

void Network::SetQueuedStaticIp(const uint32_t nStaticIp, const uint32_t nNetmask) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR ", nNetmask=" IPSTR, IP2STR(nStaticIp), IP2STR(nNetmask));

	if (nStaticIp != 0) {
		m_QueuedConfig.nStaticIp = nStaticIp;
	} else {
		m_QueuedConfig.nStaticIp = GetIp();
	}

	if (nNetmask != 0) {
		m_QueuedConfig.nNetmask = nNetmask;
	} else {
		m_QueuedConfig.nNetmask = GetNetmask();
	}

	m_QueuedConfig.nMask |= QueuedConfig::STATIC_IP;
	m_QueuedConfig.nMask |= QueuedConfig::NETMASK;

	DEBUG_EXIT
}

void Network::SetQueuedDefaultRoute(const uint32_t nGatewayIp) {
	if (nGatewayIp != 0) {
		m_QueuedConfig.nGateway = nGatewayIp;
	} else {
		m_QueuedConfig.nGateway = GetGatewayIp();
	}

	m_QueuedConfig.nMask |= QueuedConfig::GW;
}

bool Network::ApplyQueuedConfig() {
	DEBUG_ENTRY
	DEBUG_PRINTF("m_QueuedConfig.nMask=%x, " IPSTR ", " IPSTR, m_QueuedConfig.nMask, IP2STR(m_QueuedConfig.nStaticIp), IP2STR(m_QueuedConfig.nNetmask));

	if (m_QueuedConfig.nMask == QueuedConfig::NONE) {
		DEBUG_EXIT
		return false;
	}

	if ((isQueuedMaskSet(QueuedConfig::STATIC_IP)) || (isQueuedMaskSet(QueuedConfig::NETMASK)) || (isQueuedMaskSet(QueuedConfig::GW))) {
		// After SetIp all ip address might be zero.
		if (isQueuedMaskSet(QueuedConfig::STATIC_IP)) {
			SetIp(m_QueuedConfig.nStaticIp);
		}

		if (isQueuedMaskSet(QueuedConfig::NETMASK)) {
			SetNetmask(m_QueuedConfig.nNetmask);
		}

		if (isQueuedMaskSet(QueuedConfig::GW)) {
			SetGatewayIp(m_QueuedConfig.nGateway);
		}

		m_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	if (isQueuedMaskSet(QueuedConfig::DHCP)) {
		if (m_QueuedConfig.mode == network::dhcp::Mode::ACTIVE) {
			EnableDhcp();
		} else if (m_QueuedConfig.mode == network::dhcp::Mode::INACTIVE) {

		}

		m_QueuedConfig.mode = network::dhcp::Mode::UNKNOWN;
		m_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	if (isQueuedMaskSet(QueuedConfig::ZEROCONF)) {
		SetZeroconf();
		m_QueuedConfig.nMask = QueuedConfig::NONE;

		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return false;
}

void Network::Print() {
	printf("Network [%c]\n", GetAddressingMode());
	printf(" Hostname  : %s\n", m_aHostName);
	printf(" IfName    : %u: %s " MACSTR "\n", static_cast<unsigned int>(GetIfIndex()), m_aIfName, MAC2STR(net::netif_hwaddr()));
	printf(" Primary   : " IPSTR "/%u (HTTP only " IPSTR ")\n", IP2STR(net::netif_ipaddr()), static_cast<unsigned int>(GetNetmaskCIDR()), IP2STR(net::netif_secondary_ipaddr()));
	printf(" Gateway   : " IPSTR "\n", IP2STR(net::netif_gw()));
	printf(" Broadcast : " IPSTR "\n", IP2STR(GetBroadcastIp()));
}
