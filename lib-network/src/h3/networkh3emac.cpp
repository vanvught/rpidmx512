/**
 * networkh3emac.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "debug.h"

#include "networkh3emac.h"
#include "networkparams.h"

#include "./../lib-h3/include/net/net.h"

#include "hardwarebaremetal.h"

#define TO_HEX(i)		((i) < 10) ? (char)'0' + (char)(i) : (char)'A' + (char)((i) - 10)

#define HOST_NAME_PREFIX	"allwinner_"

extern "C" {
int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);
// MAC-PHY
int emac_start(bool reset_emac);
// Net
}

NetworkH3emac::NetworkH3emac(void) {
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

	emac_start(params.GetResetEmac());

	hardware_get_mac_address((uint8_t *) m_aNetMacaddr);

	tIpInfo.ip.addr = params.GetIpAddress();
	tIpInfo.netmask.addr = params.GetNetMask();
	tIpInfo.gw.addr = params.GetDefaultGateway();

	m_IsDhcpUsed = params.isDhcpUsed();

	const uint8_t *p = (const uint8_t *) params.GetHostName();

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
		strncpy(m_aHostName, (const char *) p, sizeof(m_aHostName) - 1);
	}

	net_init((const uint8_t *) m_aNetMacaddr, &tIpInfo, (const uint8_t *) m_aHostName, m_IsDhcpUsed);

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

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

void NetworkH3emac::MacAddressCopyTo(uint8_t* pMacAddress) {
	DEBUG_ENTRY

	for (unsigned i =  0; i < NETWORK_MAC_SIZE; i++) {
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

uint16_t NetworkH3emac::RecvFrom(uint32_t nHandle, uint8_t* packet, uint16_t size, uint32_t* from_ip, uint16_t* from_port) {
	return udp_recv(nHandle, packet, size, from_ip, from_port);
}

void NetworkH3emac::SendTo(uint32_t nHandle, const uint8_t* packet, uint16_t size, uint32_t to_ip, uint16_t remote_port) {
	udp_send(nHandle, packet, size, to_ip, remote_port);
}

void NetworkH3emac::SetIp(uint32_t nIp) {
	DEBUG_ENTRY

	net_set_ip(nIp);

    m_IsDhcpUsed = false;
    m_nLocalIp = nIp;

	DEBUG_EXIT
}

void NetworkH3emac::Run(void) {
	net_handle();
}
