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

Network *Network::s_pThis = nullptr;

Network::Network() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_aNetMacaddr[0] = '\0';
	m_aHostName[0] = '\0';
	m_aDomainName[0] = '\0';
	m_aIfName[0] = '\0';
}

void Network::Shutdown() {
	DEBUG_ENTRY

	DEBUG_EXIT
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

void Network::SetQueuedDhcp() {
	DEBUG_ENTRY

	m_QueuedConfig.nMask |= QueuedConfig::DHCP;

	DEBUG_EXIT
}

void Network::SetQueuedZeroconf() {
	DEBUG_ENTRY

	m_QueuedConfig.nMask |= QueuedConfig::ZEROCONF;

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

uint32_t Network::CIDRToNetmask(uint8_t nCDIR) {
	if (nCDIR != 0) {
		const auto nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - nCDIR));
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
