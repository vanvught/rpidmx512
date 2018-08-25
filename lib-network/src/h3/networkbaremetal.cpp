/**
 * @file networkbaremetal.cpp
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
#include <assert.h>

#include "networkbaremetal.h"

#include "wifi.h"
#include "wifi_udp.h"

#include "util.h"

extern "C" {
	int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);
}

NetworkBaremetal::NetworkBaremetal(void): m_IsInitDone(false), _hostname(0) {
}

NetworkBaremetal::~NetworkBaremetal(void) {
	End();
}

void NetworkBaremetal::Begin(uint16_t nPort) {
	wifi_udp_begin(nPort);
}

void NetworkBaremetal::End(void) {
	m_IsInitDone = false;

	_hostname[0] = '\0';
	m_nLocalIp = 0;
	m_nNetmask = 0;
	m_nBroadcastIp = 0;
}

void NetworkBaremetal::Init(void) {
	struct ip_info info;

	if (!wifi(&info)) {
		for (;;)
			;
	}

	_hostname = (char *)wifi_get_hostname();
	(void) wifi_get_macaddr(m_aNetMacaddr);

	m_nLocalIp = info.ip.addr;
	m_nNetmask = info.netmask.addr;

	m_IsDhcpUsed = wifi_station_is_dhcp_used();

	m_IsInitDone = true;
}

const char* NetworkBaremetal::GetHostName(void) {
	return _hostname;
}

void NetworkBaremetal::MacAddressCopyTo(uint8_t* pMacAddress) {
	assert(pMacAddress != 0);

	if (m_IsInitDone) {
		memcpy((void *)pMacAddress, m_aNetMacaddr , NETWORK_MAC_SIZE);
	} else {
		hardware_get_mac_address((uint8_t *) pMacAddress);
	}
}

void NetworkBaremetal::JoinGroup(uint32_t nIp) {
	wifi_udp_joingroup(nIp);
}

uint16_t NetworkBaremetal::RecvFrom(uint8_t* packet, uint16_t size,	uint32_t* from_ip, uint16_t* from_port) {
	return wifi_udp_recvfrom(packet, size, from_ip, from_port);
}

void NetworkBaremetal::SendTo(const uint8_t* packet, uint16_t size, uint32_t to_ip, uint16_t remote_port) {
	wifi_udp_sendto(packet, size, to_ip, remote_port);
}

void NetworkBaremetal::SetIp(uint32_t nIp) {
    m_IsDhcpUsed = false;
    m_nLocalIp = nIp;
}
