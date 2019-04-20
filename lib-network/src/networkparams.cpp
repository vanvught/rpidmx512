/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "networkparams.h"
#include "network.h"
#include "networkconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#if !defined (H3)
static const char PARAMS_NAME_SERVER[] ALIGNED = "name_server";
#endif

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	uint8_t *p = (uint8_t *) &m_tNetworkParams;

	for (uint32_t i = 0; i < sizeof(struct TNetworkParams); i++) {
		*p++ = 0;
	}

	m_tNetworkParams.bIsDhcpUsed = true;
}

NetworkParams::~NetworkParams(void) {
	m_tNetworkParams.nSetList = 0;
}

bool NetworkParams::Load(void) {
	m_tNetworkParams.nSetList = 0;

	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);

	if (configfile.Read(NetworkConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pNetworkParamsStore != 0) {
			m_pNetworkParamsStore->Update(&m_tNetworkParams);
		}
	} else if (m_pNetworkParamsStore != 0) {
		m_pNetworkParamsStore->Copy(&m_tNetworkParams);
	} else {
		return false;
	}

	return true;
}

void NetworkParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pNetworkParamsStore != 0);

	if (m_pNetworkParamsStore == 0) {
		return;
	}

	m_tNetworkParams.nSetList = 0;

	ReadConfigFile config(NetworkParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pNetworkParamsStore->Update(&m_tNetworkParams);
}

void NetworkParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint32_t value32;
	uint8_t len;

	if (Sscan::Uint8(pLine, NetworkConst::PARAMS_USE_DHCP, &value8) == SSCAN_OK) {
		m_tNetworkParams.bIsDhcpUsed = !(value8 == 0);
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_DHCP;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_IP_ADDRESS, &value32) == SSCAN_OK) {
		m_tNetworkParams.nLocalIp = value32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_IP_ADDRESS;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_NET_MASK, &value32) == SSCAN_OK) {
		m_tNetworkParams.nNetmask = value32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NET_MASK;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_DEFAULT_GATEWAY, &value32) == SSCAN_OK) {
		m_tNetworkParams.nGatewayIp = value32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_DEFAULT_GATEWAY;
		return;
	}

	len = NETWORK_HOSTNAME_SIZE - 1;
	if (Sscan::Char(pLine, NetworkConst::PARAMS_HOSTNAME, (char *) m_tNetworkParams.aHostName, &len) == SSCAN_OK) {
		m_tNetworkParams.aHostName[len] = '\0';
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_HOSTNAME;
		return;
	}

	if (Sscan::Uint8(pLine, NetworkConst::PARAMS_RESET_EMAC, &value8) == SSCAN_OK) {
		m_tNetworkParams.bResetEmac = (value8 != 0);
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_EMAC;
		return;
	}

#if !defined (H3)
	if (Sscan::IpAddress(pLine, PARAMS_NAME_SERVER, &value32) == SSCAN_OK) {
		m_tNetworkParams.nNameServerIp = value32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NAME_SERVER;
		return;
	}
#endif
}

void NetworkParams::Dump(void) {
#ifndef NDEBUG
	if (m_tNetworkParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NetworkConst::PARAMS_FILE_NAME);

	if (isMaskSet(NETWORK_PARAMS_MASK_DHCP)) {
		printf(" %s=%d [%s]\n", NetworkConst::PARAMS_USE_DHCP, (int) m_tNetworkParams.bIsDhcpUsed, BOOL2STRING(m_tNetworkParams.bIsDhcpUsed));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_IP_ADDRESS)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_IP_ADDRESS, IP2STR(m_tNetworkParams.nLocalIp));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_NET_MASK)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_NET_MASK, IP2STR(m_tNetworkParams.nNetmask));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_DEFAULT_GATEWAY)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_DEFAULT_GATEWAY, IP2STR(m_tNetworkParams.nGatewayIp));
	}

#if !defined (H3)
	if (isMaskSet(NETWORK_PARAMS_MASK_NAME_SERVER)) {
		printf(" %s=" IPSTR "\n", PARAMS_NAME_SERVER, IP2STR(m_tNetworkParams.nNameServerIp));
	}
#endif

	if (isMaskSet(NETWORK_PARAMS_MASK_HOSTNAME)) {
		printf(" %s=%s\n", NetworkConst::PARAMS_HOSTNAME, m_tNetworkParams.aHostName);
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_EMAC)) {
		printf(" %s=%d [%s]\n", NetworkConst::PARAMS_RESET_EMAC, (int) m_tNetworkParams.bResetEmac, BOOL2STRING(m_tNetworkParams.bResetEmac));
	}
#endif
}

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((NetworkParams *) p)->callbackFunction(s);
}

bool NetworkParams::isMaskSet(uint32_t nMask) const {
	return (m_tNetworkParams.nSetList & nMask) == nMask;
}
