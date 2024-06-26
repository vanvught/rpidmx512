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
#include "networkstore.h"

#include "hardware.h"

#include "../net/net.h"
#include "../../config/net_config.h"

#include "emac/emac.h"
#include "emac/phy.h"
#include "emac/mmi.h"
#include "emac/net_link_check.h"

#include "debug.h"

namespace network {
void __attribute__((weak)) mdns_announcement() {}
void __attribute__((weak)) mdns_shutdown() {}
}  // namespace network

static constexpr char TO_HEX(const char i) {
	return static_cast<char>(((i) < 10) ? '0' + i : 'A' + (i - 10));
}

#if !defined PHY_ADDRESS
# define PHY_ADDRESS	1
#endif

Network *Network::s_pThis;

Network::Network() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aDomainName[0] = '\0';

	strcpy(m_aIfName, "eth0");

	network::display_emac_config();

	emac_config();

	network::display_emac_start();

	emac_start(m_aNetMacaddr, s_lastState);

	NetworkParams params;
	params.Load();

	m_nNtpServerIp = params.GetNtpServer();
	m_fNtpUtcOffset = params.GetNtpUtcOffset();

	net::phy_customized_timing();
	net::phy_customized_led();

	const auto *p = params.GetHostName();

	if (*p == '\0') {
		uint32_t k = 0;

		for (uint32_t i = 0; (i < (sizeof(HOST_NAME_PREFIX) - 1)) && (i < network::HOSTNAME_SIZE - 7); i++) {
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
	net::link_status_read();
#endif

	Start(s_lastState);
}

void Network::Start(const net::Link link) {
	DEBUG_PRINTF("Link %s", link == net::Link::STATE_UP ? "Up" : "Down");

	NetworkParams params;
	params.Load();

	m_IpInfo.ip.addr = params.GetIpAddress();
	m_IpInfo.netmask.addr = params.GetNetMask();
	m_IpInfo.gw.addr = params.GetDefaultGateway();
	m_IsDhcpUsed = params.isDhcpUsed();
	m_nDhcpRetryTime = params.GetDhcpRetryTime();

#ifndef NDEBUG
	Print();
#endif

	network::display_emac_status(net::Link::STATE_UP == link);

	if (net::Link::STATE_UP == link) {
		if (!m_IsDhcpUsed) {
			DEBUG_PUTS("");
			if (m_IpInfo.ip.addr == 0) {
				DEBUG_PUTS("");
			} else if (!IsValidIp(m_IpInfo.gw.addr)) {
				DEBUG_PUTS("");
				m_IpInfo.gw.addr = m_IpInfo.ip.addr;
			}
		}

		if (m_IsDhcpUsed) {
			network::display_dhcp_status(network::dhcp::ClientStatus::RENEW);
		}

		net_init(m_aNetMacaddr, &m_IpInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

		if (m_IsZeroconfUsed) {
			network::display_dhcp_status(network::dhcp::ClientStatus::FAILED);
		}

		while (m_IsZeroconfUsed && (m_nDhcpRetryTime != 0) && m_IsDhcpUsed) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::FAST);

			network::display_dhcp_status(network::dhcp::ClientStatus::RETRYING);

			auto nTime = time(nullptr);
			while ((time(nullptr) - nTime) < (m_nDhcpRetryTime * 60)) {
				Hardware::Get()->Run();
			}

			network::display_dhcp_status(network::dhcp::ClientStatus::RENEW);

			Hardware::Get()->SetMode(hardware::ledblink::Mode::OFF_ON);

			m_IsDhcpUsed = true;
			m_IsZeroconfUsed = false;

			net_init(m_aNetMacaddr, &m_IpInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

			if (m_IsDhcpUsed) {
				break;
			}
		}
	} else {
		if (m_IsDhcpUsed) {
			DEBUG_PUTS("m_IsDhcpUsed=true");
			m_IpInfo.ip.addr = 0;
			m_IpInfo.netmask.addr = 0;
			m_IpInfo.gw.addr = 0;
		}

		auto bFalse = false;

		net_init(m_aNetMacaddr, &m_IpInfo, m_aHostName, &bFalse, &bFalse);
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

	m_IpInfo.ip.addr = nIp;

	if (nIp == 0) {
	} else {
		m_IpInfo.gw.addr = m_IpInfo.ip.addr;
	}

	net_set_ip(&m_IpInfo);
	net_set_gw(&m_IpInfo);

	NetworkStore::SaveIp(m_IpInfo.ip.addr);
	NetworkStore::SaveGatewayIp(m_IpInfo.gw.addr);
	NetworkStore::SaveDhcp(false);

	network::mdns_announcement();
	network::display_ip();
	network::display_netmask();

	DEBUG_EXIT
}

void Network::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (m_IpInfo.netmask.addr == nNetmask) {
		DEBUG_EXIT
		return;
	}

	m_IpInfo.netmask.addr = nNetmask;
	net_set_netmask(&m_IpInfo);

	NetworkStore::SaveNetMask(m_IpInfo.netmask.addr);

	network::display_ip();
	network::display_netmask();

	DEBUG_EXIT
}

void Network::SetGatewayIp(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (m_IpInfo.gw.addr == nGatewayIp) {
		DEBUG_EXIT
		return;
	}

	m_IpInfo.gw.addr = nGatewayIp;
	net_set_gw(&m_IpInfo);

	NetworkStore::SaveGatewayIp(m_IpInfo.gw.addr);

	network::display_gateway();

	DEBUG_EXIT
}

void Network::SetHostName(const char *pHostName) {
	DEBUG_ENTRY

	strncpy(m_aHostName, pHostName, network::HOSTNAME_SIZE - 1);
	m_aHostName[network::HOSTNAME_SIZE - 1] = '\0';

	NetworkStore::SaveHostName(m_aHostName, static_cast<uint16_t>(strlen(m_aHostName)));

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

	m_IsZeroconfUsed = net_set_zeroconf(&m_IpInfo);

	if (m_IsZeroconfUsed) {
		m_IsDhcpUsed = false;

		NetworkStore::SaveDhcp(true);// Zeroconf is enabled only when use_dhcp=1
	}

	network::mdns_announcement();
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

	m_IsDhcpUsed = net_set_dhcp(&m_IpInfo, m_aHostName, &m_IsZeroconfUsed);

	if (m_IsZeroconfUsed) {
		network::display_dhcp_status(network::dhcp::ClientStatus::FAILED);
	} else {
		network::display_dhcp_status(network::dhcp::ClientStatus::GOT_IP);
	}

	DEBUG_PRINTF("m_IsDhcpUsed=%d, m_IsZeroconfUsed=%d", m_IsDhcpUsed, m_IsZeroconfUsed);

	NetworkStore::SaveDhcp(m_IsDhcpUsed);

	network::mdns_announcement();
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
	printf("Network [%c]\n", GetAddressingMode());
	printf(" Hostname  : %s\n", m_aHostName);
	printf(" IfName    : %u: %s " MACSTR "\n", static_cast<unsigned int>(m_nIfIndex), m_aIfName, MAC2STR(m_aNetMacaddr));
	printf(" Primary   : " IPSTR "/%u (HTTP only " IPSTR ")\n", IP2STR(m_IpInfo.ip.addr), static_cast<unsigned int>(GetNetmaskCIDR()), IP2STR(m_IpInfo.secondary_ip.addr));
	printf(" Gateway   : " IPSTR "\n", IP2STR(m_IpInfo.gw.addr));
	printf(" Broadcast : " IPSTR "\n", IP2STR(GetBroadcastIp()));
}
