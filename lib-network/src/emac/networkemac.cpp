/**
 * networkemac.h
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

#include "networkemac.h"
#include "networkparams.h"

#include "hardware.h"
#include "ledblink.h"

#include "./../net/net.h"

#include "debug.h"

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

#define HOST_NAME_PREFIX	"allwinner_"

extern "C" {
// MAC-PHY
int emac_start(uint8_t paddr[]);
}

NetworkEmac::NetworkEmac() {
	DEBUG_ENTRY

	strcpy(m_aIfName, "eth0");

	DEBUG_EXIT
}

void NetworkEmac::Init(NetworkParamsStore *pNetworkParamsStore) {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	NetworkParams params(pNetworkParamsStore);

	if (params.Load()) {
		params.Dump();
	}
	
	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowEmacStart();
	}

	emac_start(m_aNetMacaddr);

	tIpInfo.ip.addr = params.GetIpAddress();
	tIpInfo.netmask.addr = params.GetNetMask();
	tIpInfo.gw.addr = params.GetDefaultGateway();

	m_IsDhcpUsed = params.isDhcpUsed();
	m_nNtpServerIp = params.GetNtpServer();
	m_fNtpUtcOffset = params.GetNtpUtcOffset();

	const char *p = params.GetHostName();

	if (*p == '\0') {
		unsigned k = 0;

		for (unsigned i = 0; (HOST_NAME_PREFIX[i] != 0) && (i < NETWORK_HOSTNAME_SIZE - 7); i++) {
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

	if ((m_pNetworkDisplay != nullptr) && m_IsDhcpUsed) {
		m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::RENEW);
	}

	net_init(m_aNetMacaddr, &tIpInfo, reinterpret_cast<const uint8_t*>(m_aHostName), &m_IsDhcpUsed, &m_IsZeroconfUsed);

	if ((m_pNetworkDisplay != nullptr) && m_IsZeroconfUsed) {
		m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::FAILED);
	}

	const auto nRetryTime = params.GetDhcpRetryTime();
	const auto bUseDhcp = params.isDhcpUsed();

	while (m_IsZeroconfUsed && (nRetryTime != 0) && bUseDhcp) {
		LedBlink::Get()->SetMode(ledblink::Mode::FAST);

		if (m_pNetworkDisplay != nullptr) {
			m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::RETRYING);
		}
		DEBUG_PUTS("");
		auto nTime = time(nullptr);
		while ((time(nullptr) - nTime) < (nRetryTime * 60)) {
			LedBlink::Get()->Run();
		}

		if (m_pNetworkDisplay != nullptr) {
			m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::RENEW);
		}

		LedBlink::Get()->SetMode(ledblink::Mode::OFF_ON);

		m_IsDhcpUsed = true;
		m_IsZeroconfUsed = false;

		net_init(m_aNetMacaddr, &tIpInfo, reinterpret_cast<const uint8_t*>(m_aHostName), &m_IsDhcpUsed, &m_IsZeroconfUsed);

		if (m_IsDhcpUsed) {
			break;
		}
	}

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	DEBUG_EXIT
}

void NetworkEmac::Shutdown() {
	DEBUG_ENTRY

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowShutdown();
	}

	net_shutdown();

	DEBUG_EXIT
}

int32_t NetworkEmac::Begin(uint16_t nPort) {
	DEBUG_ENTRY

	const int32_t nIdx = udp_bind(nPort);

	assert(nIdx != -1);

	return nIdx;

	DEBUG_EXIT
}

int32_t NetworkEmac::End(uint16_t nPort) {
	DEBUG_ENTRY

	const int32_t n = udp_unbind(nPort);

	assert(n == 0);

	return n;

	DEBUG_EXIT
}

void NetworkEmac::MacAddressCopyTo(uint8_t *pMacAddress) {
	DEBUG_ENTRY

	for (uint32_t i =  0; i < NETWORK_MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}

	DEBUG_EXIT
}

void NetworkEmac::JoinGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_join(nIp);

	DEBUG_EXIT
}

void NetworkEmac::LeaveGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_leave(nIp);

	DEBUG_EXIT
}

uint16_t NetworkEmac::RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *from_ip, uint16_t *from_port) {
	return udp_recv(static_cast<uint8_t>(nHandle), reinterpret_cast<uint8_t*>(pBuffer), nLength, from_ip, from_port);
}

void NetworkEmac::SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t to_ip, uint16_t remote_port) {
	udp_send(static_cast<uint8_t>(nHandle), reinterpret_cast<const uint8_t*>(pBuffer), nLength, to_ip, remote_port);
}

void NetworkEmac::SetDefaultIp() {
	DEBUG_ENTRY

	m_nLocalIp = 2
			+ ((static_cast<uint32_t>(m_aNetMacaddr[3])) << 8)
			+ ((static_cast<uint32_t>(m_aNetMacaddr[4])) << 16)
			+ ((static_cast<uint32_t>(m_aNetMacaddr[5])) << 24);
	m_nNetmask = 255;
	m_nGatewayIp = m_nLocalIp;

	DEBUG_EXIT
}

void NetworkEmac::SetIp(uint32_t nIp) {
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

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowIp();
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowNetMask();
	}

	DEBUG_EXIT
}

void NetworkEmac::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (m_nNetmask == nNetmask) {
		DEBUG_EXIT
		return;
	}

	m_nNetmask = nNetmask;

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveNetMask(nNetmask);
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowIp();
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowNetMask();
	}

	DEBUG_EXIT
}

void NetworkEmac::SetGatewayIp(uint32_t nGatewayIp) {
	DEBUG_ENTRY

	if (m_nGatewayIp == nGatewayIp) {
		DEBUG_EXIT
		return;
	}

	net_set_gw(nGatewayIp);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveGatewayIp(nGatewayIp);
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowGatewayIp();
	}

	DEBUG_EXIT
}

void NetworkEmac::SetHostName(const char *pHostName) {
	Network::SetHostName(pHostName);

	net_set_hostname(pHostName);

	if (m_pNetworkStore != nullptr) {
		m_pNetworkStore->SaveHostName(pHostName, static_cast<uint16_t>(strlen(pHostName)));
	}
}

bool NetworkEmac::SetZeroconf() {
	struct ip_info tIpInfo;

	m_IsZeroconfUsed = net_set_zeroconf(&tIpInfo);

	if (m_IsZeroconfUsed) {
		m_nLocalIp = tIpInfo.ip.addr;
		m_nNetmask = tIpInfo.netmask.addr;
		m_nGatewayIp = tIpInfo.gw.addr;

		m_IsDhcpUsed = false;

		if (m_pNetworkStore != nullptr) {
			m_pNetworkStore->SaveDhcp(true);	// Zeroconf is enabled only when use_dhcp=1
		}
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowIp();
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowNetMask();
	}

	return m_IsZeroconfUsed;
}

bool NetworkEmac::EnableDhcp() {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	const bool bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::RENEW);
	}

	m_IsDhcpUsed = net_set_dhcp(&tIpInfo, &m_IsZeroconfUsed);

	if (m_pNetworkDisplay != nullptr) {
		if (m_IsZeroconfUsed) {
			m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::FAILED);
		} else {
			m_pNetworkDisplay->ShowDhcpStatus(DhcpClientStatus::GOT_IP);
		}
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

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowIp();
	}

	if (m_pNetworkDisplay != nullptr) {
		m_pNetworkDisplay->ShowNetMask();
	}

	DEBUG_EXIT
	return m_IsDhcpUsed;
}
