/**
 * networkh3emac.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <assert.h>

#include "debug.h"

#include "networkh3emac.h"
#include "networkparams.h"

#include "networkdisplay.h"
#include "networkstore.h"

#include "hardware.h"

#include "./../lib-h3/include/net/net.h"

#define TO_HEX(i)		((i) < 10) ? '0' + (i) : 'A' + ((i) - 10)

#define HOST_NAME_PREFIX	"allwinner_"

extern "C" {
int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);
// MAC-PHY
int emac_start(bool reset_emac);
}

NetworkH3emac::NetworkH3emac(void) {
	strcpy(m_aIfName, "eth0");
}

NetworkH3emac::~NetworkH3emac(void) {
}

int NetworkH3emac::Init(NetworkParamsStore *pNetworkParamsStore) {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	NetworkParams params(pNetworkParamsStore);

	if (params.Load()) {
		params.Dump();
	}

	emac_start(true);

	hardware_get_mac_address(m_aNetMacaddr);

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

	net_init(m_aNetMacaddr, &tIpInfo, (const uint8_t *) m_aHostName, &m_IsDhcpUsed);

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	if (m_nGatewayIp == 0) {
		m_nGatewayIp = m_nLocalIp;
	}

	DEBUG_EXIT

	return 0;
}

int32_t NetworkH3emac::Begin(uint16_t nPort) {
	DEBUG_ENTRY

	const int32_t nIdx = udp_bind(nPort);

	assert(nIdx != -1);

	return nIdx;

	DEBUG_EXIT
}

int32_t NetworkH3emac::End(uint16_t nPort) {
	DEBUG_ENTRY

	const int32_t n = udp_unbind(nPort);

	assert(n == 0);

	return n;

	DEBUG_EXIT
}

void NetworkH3emac::MacAddressCopyTo(uint8_t *pMacAddress) {
	DEBUG_ENTRY

	for (uint32_t i =  0; i < NETWORK_MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}

	DEBUG_EXIT
}

void NetworkH3emac::JoinGroup(uint32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_join(nIp);

	DEBUG_EXIT
}

void NetworkH3emac::LeaveGroup(uint32_t nHandle, uint32_t nIp) {
	DEBUG_ENTRY

	igmp_leave(nIp);

	DEBUG_EXIT
}

uint16_t NetworkH3emac::RecvFrom(uint32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *from_ip, uint16_t *from_port) {
	return udp_recv(nHandle, reinterpret_cast<uint8_t*>(pBuffer), nLength, from_ip, from_port);
}

void NetworkH3emac::SendTo(uint32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t to_ip, uint16_t remote_port) {
	udp_send(nHandle, reinterpret_cast<const uint8_t*>(pBuffer), nLength, to_ip, remote_port);
}

void NetworkH3emac::SetIp(uint32_t nIp) {
	DEBUG_ENTRY

	if (nIp == m_nLocalIp) {
		return;
	}

	if (nIp == 0) {
		struct ip_info tIpInfo;
		net_set_default_ip(&tIpInfo);

		m_nLocalIp = tIpInfo.ip.addr;
		m_nNetmask = tIpInfo.netmask.addr;
		m_nGatewayIp = tIpInfo.ip.addr; //tIpInfo.gw.addr; There is no gateway support
	} else {
		net_set_ip(nIp);
		m_nLocalIp = nIp;
		m_IsDhcpUsed = false;

		if (m_pNetworkStore != 0) {
			m_pNetworkStore->SaveIp(nIp);
			m_pNetworkStore->SaveDhcp(false);
		}
	}

	if (m_pNetworkDisplay != 0) {
		m_pNetworkDisplay->ShowIp();
	}

	DEBUG_EXIT
}

void NetworkH3emac::SetNetmask(uint32_t nNetmask) {
	DEBUG_ENTRY

	if (m_nNetmask == nNetmask) {
		return;
	}

	m_nNetmask = nNetmask;

	if (m_pNetworkStore != 0) {
		m_pNetworkStore->SaveNetMask(nNetmask);
	}

	if (m_pNetworkDisplay != 0) {
		m_pNetworkDisplay->ShowNetMask();
	}

	DEBUG_EXIT
}

void NetworkH3emac::SetHostName(const char *pHostName) {
	Network::SetHostName(pHostName);

	net_set_hostname(pHostName);

	if (m_pNetworkStore != 0) {
		m_pNetworkStore->SaveHostName(pHostName, strlen(pHostName));
	}
}

bool NetworkH3emac::EnableDhcp(void) {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	bool bWatchdog = Hardware::Get()->IsWatchdog();

	if (bWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	m_IsDhcpUsed =  net_set_dhcp(&tIpInfo);

	if (bWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_PRINTF("m_IsDhcpUsed=%d", m_IsDhcpUsed);

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	if (m_pNetworkStore != 0) {
		m_pNetworkStore->SaveDhcp(m_IsDhcpUsed);
	}

	if (m_pNetworkDisplay != 0) {
		m_pNetworkDisplay->ShowIp();
	}

	if (m_pNetworkDisplay != 0) {
		m_pNetworkDisplay->ShowNetMask();
	}

	DEBUG_EXIT
	return m_IsDhcpUsed;
}
