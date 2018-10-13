/**
 * networkh3emac.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "debug.h"

#include "networkh3emac.h"
#include "networkparams.h"

#include "./../lib-h3/include/net/net.h"

#include "hardwarebaremetal.h"

#include "util.h"

#define HOST_NAME_PREFIX	"allwinner_"

extern "C" {
int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);
// MAC-PHY
int emac_start(void);
// Net
}

NetworkH3emac::NetworkH3emac(void) : m_nIdx(-1) {
	uint8_t i;

	for (i = 0; i < NETWORK_HOSTNAME_SIZE; i++) {
		m_aHostname[i] = 0;
	}

	emac_start();
}

NetworkH3emac::~NetworkH3emac(void) {
	End();
}

int NetworkH3emac::Init(NetworkParamsStore *pNetworkParamsStore) {
	DEBUG_ENTRY

	struct ip_info tIpInfo;

	NetworkParams params(pNetworkParamsStore);

	if (!Hardware::Get()->IsButtonPressed()) {
		if (params.Load()) {
			params.Dump();
		}
	}

	hardware_get_mac_address((uint8_t *) m_aNetMacaddr);

	tIpInfo.ip.addr = params.GetIpAddress();
	tIpInfo.netmask.addr = params.GetNetMask();
	tIpInfo.gw.addr = params.GetDefaultGateway();

	m_IsDhcpUsed = params.isDhcpUsed();

	const uint8_t *p = (const uint8_t *) params.GetHostName();

	if (*p == '\0') {
		uint8_t i;
		uint8_t k = 0;
		for (i = 0; (HOST_NAME_PREFIX[i] != 0) && (i < NETWORK_HOSTNAME_SIZE - 7); i++) {
			m_aHostname[k++] = HOST_NAME_PREFIX[i];
		}
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[3] >> 4);
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[3] & 0x0F);
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[4] >> 4);
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[4] & 0x0F);
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[5] >> 4);
		m_aHostname[k++] = TO_HEX(m_aNetMacaddr[5] & 0x0F);
		m_aHostname[k] = '\0';
	} else {
		strncpy(m_aHostname, (const char *) p, NETWORK_HOSTNAME_SIZE);
	}

	net_init((const uint8_t *) m_aNetMacaddr, &tIpInfo, (const uint8_t *) m_aHostname, m_IsDhcpUsed);

	m_nLocalIp = tIpInfo.ip.addr;
	m_nNetmask = tIpInfo.netmask.addr;
	m_nGatewayIp = tIpInfo.gw.addr;

	DEBUG_EXIT

	return 0;
}

void NetworkH3emac::Begin(uint16_t nPort) {
	DEBUG_ENTRY

	m_nIdx = udp_bind(nPort);

	assert(m_nIdx != -1);

	DEBUG_EXIT
}

void NetworkH3emac::End(void) {
}

void NetworkH3emac::MacAddressCopyTo(uint8_t* pMacAddress) {
	DEBUG_ENTRY

	for (unsigned i =  0; i < NETWORK_MAC_SIZE; i++) {
		pMacAddress[i] = m_aNetMacaddr[i];
	}

	DEBUG_EXIT
}

void NetworkH3emac::JoinGroup(uint32_t nIp) {
	DEBUG_ENTRY

	igmp_join(nIp);

	DEBUG_EXIT
}

uint16_t NetworkH3emac::RecvFrom(uint8_t* packet, uint16_t size, uint32_t* from_ip, uint16_t* from_port) {
	return udp_recv(m_nIdx, packet, size, from_ip, from_port);
}

void NetworkH3emac::SendTo(const uint8_t* packet, uint16_t size, uint32_t to_ip, uint16_t remote_port) {
	udp_send(m_nIdx, packet, size, to_ip, remote_port);
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
