/**
 * @file network.c
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

#include "network.h"

#include "debug.h"

Network *Network::s_pThis = 0;

Network::Network(void) :
	m_nLocalIp(0),
	m_nGatewayIp(0),
	m_nNetmask(0),
	m_IsDhcpCapable(true),
	m_IsDhcpUsed(false),
	m_nIfIndex(1),
	m_nNtpServerIp(0),
	m_fNtpUtcOffset(0),
	m_pNetworkDisplay(0),
	m_pNetworkStore(0),
	m_nQueuedLocalIp(0),
	m_nQueuedNetmask(0)
{
	assert(s_pThis == 0);
	s_pThis = this;

	m_aNetMacaddr[0] = '\0';
	m_aHostName[0] = '\0';
	m_aDomainName[0] = '\0';
	m_aIfName[0] = '\0';
}

Network::~Network(void) {
	s_pThis = 0;
}

bool Network::SetStaticIp(bool bQueueing, uint32_t nLocalIp, uint32_t nNetmask) {
	DEBUG_PRINTF("bQueueing=%d, nLocalIp=" IPSTR ", nNetmask=" IPSTR, static_cast<int>(bQueueing), IP2STR(nLocalIp), IP2STR(nNetmask));

	if (bQueueing) {
		m_nQueuedLocalIp = nLocalIp;
		m_nQueuedNetmask = nNetmask;
		return true;
	}

	if (m_nQueuedLocalIp != 0) {

		SetIp(m_nQueuedLocalIp);
		SetNetmask(m_nQueuedNetmask);

		m_nQueuedLocalIp = 0;
		m_nQueuedNetmask = 0;

		return true;
	}

	return false;
}

uint32_t Network::CIDRToNetmask(uint8_t nCDIR) {
	if (nCDIR != 0) {
		const uint32_t nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - nCDIR));
		DEBUG_PRINTF("%d " IPSTR, nCDIR, IP2STR(nNetmask));
		return nNetmask;
	}

	return 0;
}

void Network::SetHostName(const char *pHostName) {
	DEBUG_ENTRY

	strncpy(m_aHostName, pHostName, NETWORK_HOSTNAME_SIZE - 1);
	m_aHostName[NETWORK_HOSTNAME_SIZE - 1] = '\0';

	DEBUG_PUTS(m_aHostName);
	DEBUG_EXIT
}

void Network::SetDomainName(const char *pDomainName) {
	DEBUG_ENTRY

	strncpy(m_aDomainName, pDomainName, NETWORK_DOMAINNAME_SIZE - 1);
	m_aDomainName[NETWORK_DOMAINNAME_SIZE - 1] = '\0';

	DEBUG_PUTS(m_aDomainName);
	DEBUG_EXIT
}

bool Network::EnableDhcp(void) {
	DEBUG_PUTS("false");
	return false;
}
