/**
 * network.cpp
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
#include <time.h>
#include <cassert>

#include "network.h"
#include "networkparams.h"

#include "hardware.h"

#include "../net/net.h"
#include "../../config/net_config.h"
#include "emac/net_link_check.h"

#include "debug.h"

namespace net {
void __attribute__((weak)) phy_customized_led() {}
void __attribute__((weak)) phy_customized_timing() {}
}  // namespace net

namespace network {
void __attribute__((weak)) mdns_announcement() {}
}  // namespace network

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

int emac_start(uint8_t paddr[]);

Network *Network::s_pThis;

Network::Network(NetworkParamsStore *pNetworkParamsStore) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aDomainName[0] = '\0';

	strcpy(m_aIfName, "eth0");

	network::display_emac_start();

	emac_start(m_aNetMacaddr);

	NetworkParams params(pNetworkParamsStore);

	if (params.Load()) {
		params.Dump();
	}

	m_ipInfo.ip.addr = params.GetIpAddress();
	m_ipInfo.netmask.addr = params.GetNetMask();
	m_ipInfo.gw.addr = params.GetDefaultGateway();
	m_IsDhcpUsed = params.isDhcpUsed();
	m_nNtpServerIp = params.GetNtpServer();
	m_fNtpUtcOffset = params.GetNtpUtcOffset();

	net::phy_customized_timing();
	net::phy_customized_led();

	const auto *p = params.GetHostName();

	if (*p == '\0') {
		uint32_t k = 0;

		for (uint32_t i = 0; (HOST_NAME_PREFIX[i] != 0) && (i < network::HOSTNAME_SIZE - 7); i++) {
			m_aHostName[k++] = HOST_NAME_PREFIX[i];
		}

		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[3] >> 4);
		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[3] & 0x0F);
		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[4] >> 4);
		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[4] & 0x0F);
		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[5] >> 4);
		m_aHostName[k++] = TO_HEX(m_aNetMacaddr[5] & 0x0F);
		m_aHostName[k] = '\0';
	} else {
		strncpy(m_aHostName, p, sizeof(m_aHostName) - 1);
		m_aHostName[sizeof(m_aHostName) - 1] = '\0';
	}

#if defined (ENET_LINK_CHECK_USE_INT)
	net::link_interrupt_init();
#elif defined (ENET_LINK_CHECK_USE_PIN_POLL)
	net::link_pin_poll_init();
#elif defined (ENET_LINK_CHECK_REG_POLL)
	net::link_register_read();
#endif

	s_lastState = net::link_register_read();

	if (net::Link::STATE_UP == s_lastState) {
		DEBUG_PUTS("net::Link::STATE_UP");

		if (!m_IsDhcpUsed) {
			DEBUG_PUTS("");
			if (m_ipInfo.ip.addr == 0) {
				DEBUG_PUTS("");
				SetDefaultIp();
			} else if (!IsValidIp(m_ipInfo.gw.addr)) {
				DEBUG_PUTS("");
				m_ipInfo.gw.addr = m_ipInfo.ip.addr;
			}
		}

		if (m_IsDhcpUsed) {
			network::display_dhcp_status(network::dhcp::ClientStatus::RENEW);
		}

		net_init(m_aNetMacaddr, &m_ipInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

		if (m_IsZeroconfUsed) {
			network::display_dhcp_status(network::dhcp::ClientStatus::FAILED);
		}

		const auto nRetryTime = params.GetDhcpRetryTime();
		const auto bUseDhcp = params.isDhcpUsed();

		while (m_IsZeroconfUsed && (nRetryTime != 0) && bUseDhcp) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::FAST);

			network::display_dhcp_status(network::dhcp::ClientStatus::RETRYING);

			DEBUG_PUTS("");
			auto nTime = time(nullptr);
			while ((time(nullptr) - nTime) < (nRetryTime * 60)) {
				Hardware::Get()->Run();
			}

			network::display_dhcp_status(network::dhcp::ClientStatus::RENEW);

			Hardware::Get()->SetMode(hardware::ledblink::Mode::OFF_ON);

			m_IsDhcpUsed = true;
			m_IsZeroconfUsed = false;

			net_init(m_aNetMacaddr, &m_ipInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

			if (m_IsDhcpUsed) {
				break;
			}
		}
	} else {
		DEBUG_PUTS("net::Link::STATE_DOWN");

		if (m_IsDhcpUsed) {
			DEBUG_PUTS("m_IsDhcpUsed=true");
			m_ipInfo.ip.addr = 0;
			m_ipInfo.netmask.addr = 0;
			m_ipInfo.gw.addr = 0;
		}

		auto bFalse = false;

		net_init(m_aNetMacaddr, &m_ipInfo, m_aHostName, &bFalse, &bFalse);
	}

	network::display_ip();
	network::display_netmask();
	network::display_gateway();

	DEBUG_EXIT
}

void Network::SetIp(uint32_t nIp) {
	DEBUG_ENTRY

	if (m_IsDhcpUsed) {
		m_IsDhcpUsed = false;
		net_dhcp_release();
	}

	m_IsZeroconfUsed = false;

	if (nIp == 0) {
		SetDefaultIp();
	} else {
		m_ipInfo.ip.addr = nIp;
		m_ipInfo.gw.addr = m_ipInfo.ip.addr;
	}

	net_set_ip(m_ipInfo.ip.addr);
	net_set_gw(m_ipInfo.gw.addr);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveIp(m_ipInfo.ip.addr);
		m_pNetworkStore->SaveGatewayIp(m_ipInfo.gw.addr);
		m_pNetworkStore->SaveDhcp(false);
	}

	network::mdns_announcement();
	network::display_ip();
	network::display_netmask();

	DEBUG_EXIT
}

void Network::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (m_ipInfo.netmask.addr == nNetmask) {
		DEBUG_EXIT
		return;
	}

	m_ipInfo.netmask.addr = nNetmask;
	net_set_netmask(m_ipInfo.netmask.addr);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveNetMask(m_ipInfo.netmask.addr);
	}

	network::display_ip();
	network::display_netmask();

	DEBUG_EXIT
}

void Network::SetGatewayIp(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (m_ipInfo.gw.addr == nGatewayIp) {
		DEBUG_EXIT
		return;
	}

	m_ipInfo.gw.addr = nGatewayIp;
	net_set_gw(m_ipInfo.gw.addr);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveGatewayIp(m_ipInfo.gw.addr);
	}

	network::display_gateway();

	DEBUG_EXIT
}

void Network::SetHostName(const char *pHostName) {
	DEBUG_ENTRY

	strncpy(m_aHostName, pHostName, network::HOSTNAME_SIZE - 1);
	m_aHostName[network::HOSTNAME_SIZE - 1] = '\0';

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveHostName(m_aHostName, static_cast<uint16_t>(strlen(m_aHostName)));
	}

	network::mdns_announcement();
	network::display_hostname();

	DEBUG_EXIT
}

bool Network::SetZeroconf() {
	DEBUG_ENTRY

	const auto bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	m_IsZeroconfUsed = net_set_zeroconf(&m_ipInfo);

	if (m_IsZeroconfUsed) {
		m_IsDhcpUsed = false;

		if (m_pNetworkStore != nullptr) {
			m_pNetworkStore->SaveDhcp(true);// Zeroconf is enabled only when use_dhcp=1
		}
	}

	network::display_ip();
	network::display_netmask();

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
	return m_IsZeroconfUsed;
}

bool Network::EnableDhcp() {
	DEBUG_ENTRY

	const auto bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	network::display_dhcp_status(network::dhcp::ClientStatus::RENEW);

	m_IsDhcpUsed = net_set_dhcp(&m_ipInfo, m_aHostName, &m_IsZeroconfUsed);

	if (m_IsZeroconfUsed) {
		network::display_dhcp_status(network::dhcp::ClientStatus::FAILED);
	} else {
		network::display_dhcp_status(network::dhcp::ClientStatus::GOT_IP);
	}

	DEBUG_PRINTF("m_IsDhcpUsed=%d, m_IsZeroconfUsed=%d", m_IsDhcpUsed, m_IsZeroconfUsed);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveDhcp(m_IsDhcpUsed);
	}

	network::display_ip();
	network::display_netmask();
	network::display_gateway();

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
	return m_IsDhcpUsed;
}

void Network::SetQueuedStaticIp(uint32_t nLocalIp, uint32_t nNetmask) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR ", nNetmask=" IPSTR, IP2STR(nLocalIp), IP2STR(nNetmask));

	if (nLocalIp != 0) {
		m_QueuedConfig.nLocalIp = nLocalIp;
	}

	if (nNetmask != 0) {
		m_QueuedConfig.nNetmask = nNetmask;
	}

	m_QueuedConfig.nMask |= QueuedConfig::STATIC_IP;
	m_QueuedConfig.nMask |= QueuedConfig::NET_MASK;

	DEBUG_EXIT
}

bool Network::ApplyQueuedConfig() {
	DEBUG_ENTRY
	DEBUG_PRINTF("m_QueuedConfig.nMask=%x, " IPSTR ", " IPSTR, m_QueuedConfig.nMask, IP2STR(m_QueuedConfig.nLocalIp), IP2STR(m_QueuedConfig.nNetmask));

	if (m_QueuedConfig.nMask == QueuedConfig::NONE) {
		DEBUG_EXIT
		return false;
	}

	if ((isQueuedMaskSet(QueuedConfig::STATIC_IP)) || (isQueuedMaskSet(QueuedConfig::NET_MASK))) {
		if (isQueuedMaskSet(QueuedConfig::NET_MASK)) {
			SetNetmask(m_QueuedConfig.nNetmask);
			m_QueuedConfig.nMask &= ~(QueuedConfig::NET_MASK);
		}

		if (isQueuedMaskSet(QueuedConfig::STATIC_IP)) {
			SetIp(m_QueuedConfig.nLocalIp);
			m_QueuedConfig.nMask &= ~(QueuedConfig::STATIC_IP);
		}
	}

	if (isQueuedMaskSet(QueuedConfig::DHCP)) {
		EnableDhcp();
		m_QueuedConfig.nMask &= ~(QueuedConfig::DHCP);
	}

	if (isQueuedMaskSet(QueuedConfig::ZEROCONF)) {
		SetZeroconf();
		m_QueuedConfig.nMask &= ~(QueuedConfig::ZEROCONF);
	}

	DEBUG_EXIT
	return true;
}

#include <cstdio>

void Network::Print() {
	printf("Network\n");
	printf(" Hostname  : %s\n", m_aHostName);
	printf(" IfName    : %d: %s\n", m_nIfIndex, m_aIfName);
	printf(" Inet      : " IPSTR "/%d\n", IP2STR(m_ipInfo.ip.addr), GetNetmaskCIDR());
	printf(" Netmask   : " IPSTR "\n", IP2STR(m_ipInfo.netmask.addr));
	printf(" Gateway   : " IPSTR "\n", IP2STR(m_ipInfo.gw.addr));
	printf(" Broadcast : " IPSTR "\n", IP2STR(GetBroadcastIp()));
	printf(" Mac       : " MACSTR "\n", MAC2STR(m_aNetMacaddr));
	printf(" Mode      : %c\n", GetAddressingMode());
}
