/**
 * network.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <netinet/in.h>
#include <time.h>
#include <cassert>

#include "network.h"
#include "networkparams.h"

#include "hardware.h"
#include "ledblink.h"

#include "../net/net.h"
#include "../../config/net_config.h"

#include "debug.h"

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

extern "C" {
// MAC-PHY
int emac_start(uint8_t paddr[]);
}

Network *Network::s_pThis = nullptr;

Network::Network() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aNetMacaddr[0] = '\0';
	m_aHostName[0] = '\0';
	m_aDomainName[0] = '\0';

	strcpy(m_aIfName, "eth0");

	DEBUG_EXIT
}

void Network::Init(NetworkParamsStore *pNetworkParamsStore) {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	NetworkParams params(pNetworkParamsStore);

	if (params.Load()) {
		params.Dump();
	}
	
	m_NetworkDisplay.ShowEmacStart();

	emac_start(m_aNetMacaddr);

	tIpInfo.ip.addr = params.GetIpAddress();
	tIpInfo.netmask.addr = params.GetNetMask();
	tIpInfo.gw.addr = params.GetDefaultGateway();

	m_IsDhcpUsed = params.isDhcpUsed();
	m_nNtpServerIp = params.GetNtpServer();
	m_fNtpUtcOffset = params.GetNtpUtcOffset();

	const auto *p = params.GetHostName();

	if (*p == '\0') {
		unsigned k = 0;

		for (unsigned i = 0; (HOST_NAME_PREFIX[i] != 0) && (i < network::HOSTNAME_SIZE - 7); i++) {
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

	if (!m_IsDhcpUsed) {
		DEBUG_PUTS("");
		if (tIpInfo.ip.addr == 0) {
			DEBUG_PUTS("");

			SetDefaultIp();

			tIpInfo.ip.addr = m_nLocalIp;
			tIpInfo.netmask.addr = m_nNetmask;
			tIpInfo.gw.addr = m_nLocalIp;
		} else if (!IsValidIp(m_nGatewayIp)) {
			DEBUG_PUTS("");

			tIpInfo.gw.addr = m_nLocalIp;
		}
	}

	if (m_IsDhcpUsed) {
		m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::RENEW);
	}

	net_init(m_aNetMacaddr, &tIpInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

	if (m_IsZeroconfUsed) {
		m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::FAILED);
	}

	const auto nRetryTime = params.GetDhcpRetryTime();
	const auto bUseDhcp = params.isDhcpUsed();

	while (m_IsZeroconfUsed && (nRetryTime != 0) && bUseDhcp) {
		LedBlink::Get()->SetMode(ledblink::Mode::FAST);

		m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::RETRYING);

		DEBUG_PUTS("");
		auto nTime = time(nullptr);
		while ((time(nullptr) - nTime) < (nRetryTime * 60)) {
			LedBlink::Get()->Run();
		}

		m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::RENEW);

		LedBlink::Get()->SetMode(ledblink::Mode::OFF_ON);

		m_IsDhcpUsed = true;
		m_IsZeroconfUsed = false;

		net_init(m_aNetMacaddr, &tIpInfo, m_aHostName, &m_IsDhcpUsed, &m_IsZeroconfUsed);

		if (m_IsDhcpUsed) {
			break;
		}
	}

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	m_NetworkDisplay.ShowIp();

	DEBUG_EXIT
}

void Network::Shutdown() {
	DEBUG_ENTRY

	m_NetworkDisplay.ShowShutdown();

	net_shutdown();

	DEBUG_EXIT
}

int32_t Network::Begin(uint16_t nPort) {
	DEBUG_ENTRY

	const auto nIdx = udp_bind(nPort);

	assert(nIdx != -1);

	return nIdx;

	DEBUG_EXIT
}

int32_t Network::End(uint16_t nPort) {
	DEBUG_ENTRY

	const auto n = udp_unbind(nPort);

	assert(n == 0);

	return n;

	DEBUG_EXIT
}

void Network::MacAddressCopyTo(uint8_t *pMacAddress) {
	DEBUG_ENTRY

	for (uint32_t i =  0; i < network::MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}

	DEBUG_EXIT
}

void Network::JoinGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_join(nIp);

	DEBUG_EXIT
}

void Network::LeaveGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_leave(nIp);

	DEBUG_EXIT
}

void Network::SetDefaultIp() {
	DEBUG_ENTRY

	m_nLocalIp = 2
			+ ((static_cast<uint32_t>(m_aNetMacaddr[3])) << 8)
			+ ((static_cast<uint32_t>(m_aNetMacaddr[4])) << 16)
			+ ((static_cast<uint32_t>(m_aNetMacaddr[5])) << 24);
	m_nNetmask = 255;
	m_nGatewayIp = m_nLocalIp;

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
		net_set_ip(m_nLocalIp);
		// We do not store
	} else {
		net_set_ip(nIp);

		m_nLocalIp = nIp;
		m_nGatewayIp = m_nLocalIp;

		if (m_pNetworkStore != nullptr) {
			m_pNetworkStore->SaveIp(nIp);
			m_pNetworkStore->SaveDhcp(false);
		}
	}

	m_NetworkDisplay.ShowIp();
	m_NetworkDisplay.ShowNetMask();

	DEBUG_EXIT
}

void Network::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (m_nNetmask == nNetmask) {
		DEBUG_EXIT
		return;
	}

	m_nNetmask = nNetmask;

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveNetMask(nNetmask);
	}

	m_NetworkDisplay.ShowIp();
	m_NetworkDisplay.ShowNetMask();

	DEBUG_EXIT
}

void Network::SetGatewayIp(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (m_nGatewayIp == nGatewayIp) {
		DEBUG_EXIT
		return;
	}

	net_set_gw(nGatewayIp);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveGatewayIp(nGatewayIp);
	}

	m_NetworkDisplay.ShowGatewayIp();

	DEBUG_EXIT
}

void Network::SetHostName(const char *pHostName) {
	DEBUG_ENTRY

	strncpy(m_aHostName, pHostName, network::HOSTNAME_SIZE - 1);
	m_aHostName[network::HOSTNAME_SIZE - 1] = '\0';

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveHostName(m_aHostName, static_cast<uint16_t>(strlen(m_aHostName)));
	}

	m_NetworkDisplay.ShowHostName();

	DEBUG_EXIT
}

bool Network::SetZeroconf() {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	m_IsZeroconfUsed = net_set_zeroconf(&tIpInfo);

	if (m_IsZeroconfUsed) {
		m_nLocalIp = tIpInfo.ip.addr;
		m_nNetmask = tIpInfo.netmask.addr;
		m_nGatewayIp = tIpInfo.gw.addr;

		m_IsDhcpUsed = false;

		if (m_pNetworkStore != nullptr) {
			m_pNetworkStore->SaveDhcp(true);// Zeroconf is enabled only when use_dhcp=1
		}
	}

	m_NetworkDisplay.ShowIp();
	m_NetworkDisplay.ShowNetMask();

	DEBUG_EXIT
	return m_IsZeroconfUsed;
}

bool Network::EnableDhcp() {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	const bool bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::RENEW);

	m_IsDhcpUsed = net_set_dhcp(&tIpInfo, m_aHostName, &m_IsZeroconfUsed);

		if (m_IsZeroconfUsed) {
			m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::FAILED);
		} else {
			m_NetworkDisplay.ShowDhcpStatus(network::dhcp::ClientStatus::GOT_IP);
		}

	DEBUG_PRINTF("m_IsDhcpUsed=%d, m_IsZeroconfUsed=%d", m_IsDhcpUsed, m_IsZeroconfUsed);

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveDhcp(m_IsDhcpUsed);
	}

	m_NetworkDisplay.ShowIp();
	m_NetworkDisplay.ShowNetMask();

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
	printf(" Inet      : " IPSTR "/%d\n", IP2STR(m_nLocalIp), GetNetmaskCIDR());
	printf(" Netmask   : " IPSTR "\n", IP2STR(m_nNetmask));
	printf(" Gateway   : " IPSTR "\n", IP2STR(m_nGatewayIp));
	printf(" Broadcast : " IPSTR "\n", IP2STR(GetBroadcastIp()));
	printf(" Mac       : " MACSTR "\n", MAC2STR(m_aNetMacaddr));
	printf(" Mode      : %c\n", GetAddressingMode());
}
