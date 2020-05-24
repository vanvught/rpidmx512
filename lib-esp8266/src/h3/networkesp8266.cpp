/**
 * @file networkesp8266.cpp
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
#include <string.h>
#include <cassert>

#include "networkesp8266.h"

#include "wifi.h"
#include "wifi_udp.h"

extern "C" {
	int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address);
}

NetworkESP8266::NetworkESP8266(void): m_IsInitDone(false) {
	strcpy(m_aIfName, "wlan0");
}

NetworkESP8266::~NetworkESP8266(void) {
}

int32_t NetworkESP8266::Begin(uint16_t nPort) {
	wifi_udp_begin(nPort);
	return 0;
}

int32_t NetworkESP8266::End(__attribute__((unused)) uint16_t nPort) {
	return 0;
}

void NetworkESP8266::Init(void) {
	struct ip_info info;

	if (!wifi(&info)) {
		for (;;)
			;
	}

	const char *pHostName = wifi_get_hostname();
	strncpy(m_aHostName, pHostName, sizeof(m_aHostName) - 1);

	wifi_get_macaddr(m_aNetMacaddr);

	m_nLocalIp = info.ip.addr;
	m_nNetmask = info.netmask.addr;

	m_IsDhcpUsed = wifi_station_is_dhcp_used();

	m_IsInitDone = true;
}

void NetworkESP8266::MacAddressCopyTo(uint8_t* pMacAddress) {
	assert(pMacAddress != 0);

	if (m_IsInitDone) {
		memcpy(pMacAddress, m_aNetMacaddr , NETWORK_MAC_SIZE);
	} else {
		hardware_get_mac_address(pMacAddress);
	}
}

void NetworkESP8266::JoinGroup(__attribute__((unused)) int32_t nHandle, uint32_t nIp) {
	wifi_udp_joingroup(nIp);
}

uint16_t NetworkESP8266::RecvFrom(__attribute__((unused)) int32_t nHandle, void *pBuffer, uint16_t nLength,	uint32_t *from_ip, uint16_t* from_port) {
	return wifi_udp_recvfrom(reinterpret_cast<uint8_t*>(pBuffer), nLength, from_ip, from_port);
}

void NetworkESP8266::SendTo(__attribute__((unused)) int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t to_ip, uint16_t remote_port) {
	wifi_udp_sendto(reinterpret_cast<const uint8_t*>(pBuffer), nLength, to_ip, remote_port);
}
