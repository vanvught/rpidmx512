/**
 * @file network.c
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

#include <stdio.h>

#include "network.h"

Network *Network::s_pThis = 0;

Network::Network(void) :
	m_nLocalIp(0),
	m_nGatewayIp(0),
	m_nNetmask(0),
	m_nBroadcastIp(0),
	m_IsDhcpCapable(true),
	m_IsDhcpUsed(false)
{
	s_pThis = this;

	for (unsigned i = 0; i < sizeof(m_aNetMacaddr); i++) {
		m_aNetMacaddr[i] = 0;
	}
}

Network::~Network(void) {
	m_nLocalIp = 0;
	m_nGatewayIp = 0;
	m_nNetmask = 0;
	m_nBroadcastIp = 0;
	m_IsDhcpUsed = false;

	s_pThis = 0;
}

void Network::Print(void) {
	uint8_t aMacAddress[NETWORK_MAC_SIZE];
	MacAddressCopyTo(aMacAddress);

	printf("\nNetwork configuration\n");
	printf(" Hostname   : %s\n", GetHostName());
	printf(" Interface  : " IPSTR "\n", IP2STR(m_nLocalIp));
	printf(" Netmask    : " IPSTR "\n", IP2STR(m_nNetmask));
	printf(" MacAddress : " MACSTR "\n", MAC2STR(aMacAddress));
	if (IsDhcpKnown()) {
		printf(" DHCP       : %s\n", m_IsDhcpUsed ? "Yes" : "No");
	}
}
