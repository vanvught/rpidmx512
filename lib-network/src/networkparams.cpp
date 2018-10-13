/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined (BARE_METAL)
 #include "util.h"
#elif defined (__circle__)
 #include <circle/util.h>
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "networkparams.h"
#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#define BOOL2STRING(b)	(b) ? "Yes" : "No"

#define SET_IS_DHCP_MASK			(1 << 0)
#define SET_IP_ADDRESS_MASK			(1 << 1)
#define SET_NET_MASK_MASK			(1 << 2)
#define SET_DEFAULT_GATEWAY_MASK	(1 << 3)
#define SET_NAME_SERVER_MASK		(1 << 4)
#define SET_HOSTNAME_MASK			(1 << 5)

static const char PARAMS_FILE_NAME[] ALIGNED = "network.txt";
static const char PARAMS_USE_DHCP[] ALIGNED = "use_dhcp";
static const char PARAMS_IP_ADDRESS[] ALIGNED = "ip_address";
static const char PARAMS_NET_MASK[] ALIGNED = "net_mask";
static const char PARAMS_DEFAULT_GATEWAY[] ALIGNED = "default_gateway";
static const char PARAMS_NAME_SERVER[] ALIGNED = "name_server";
static const char PARAMS_HOSTNAME[] ALIGNED = "hostname";

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((NetworkParams *) p)->callbackFunction(s);
}

void NetworkParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint32_t value32;
	char value[NETWORK_HOSTNAME_SIZE];
	uint8_t len;

	if (Sscan::Uint8(pLine, PARAMS_USE_DHCP, &value8) == SSCAN_OK) {
		if (value8 == 0) {
			m_tNetworkParams.bIsDhcpUsed = false;
		}
		m_tNetworkParams.bSetList |= SET_IS_DHCP_MASK;
		return;
	}

	if (Sscan::IpAddress(pLine, PARAMS_IP_ADDRESS, &value32) == 1) {
		m_tNetworkParams.nLocalIp = value32;
		m_tNetworkParams.bSetList |= SET_IP_ADDRESS_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NET_MASK, &value32) == 1) {
		m_tNetworkParams.nNetmask = value32;
		m_tNetworkParams.bSetList |= SET_NET_MASK_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_DEFAULT_GATEWAY, &value32) == 1) {
		m_tNetworkParams.nGatewayIp = value32;
		m_tNetworkParams.bSetList |= SET_DEFAULT_GATEWAY_MASK;
	} else if (Sscan::IpAddress(pLine, PARAMS_NAME_SERVER, &value32) == 1) {
		m_tNetworkParams.nNameServerIp = value32;
		m_tNetworkParams.bSetList |= SET_NAME_SERVER_MASK;
	}

	len = NETWORK_HOSTNAME_SIZE;

	if (Sscan::Char(pLine, PARAMS_HOSTNAME, value, &len) == SSCAN_OK) {
		strncpy((char *) m_tNetworkParams.aHostName, value, len);
		m_tNetworkParams.aHostName[NETWORK_HOSTNAME_SIZE - 1] = '\0';
		m_tNetworkParams.bSetList |= SET_HOSTNAME_MASK;
		return;
	}
}

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	uint8_t *p = (uint8_t *) &m_tNetworkParams;

	for (uint32_t i = 0; i < sizeof(struct TNetworkParams); i++) {
		p[i] = 0;
	}

	m_tNetworkParams.bIsDhcpUsed = true;
}

NetworkParams::~NetworkParams(void) {
	m_tNetworkParams.bSetList = 0;
}

bool NetworkParams::Load(void) {
	m_tNetworkParams.bSetList = 0;

	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
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

void NetworkParams::Dump(void) {
#ifndef NDEBUG
	if (m_tNetworkParams.bSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_IS_DHCP_MASK)) {
		printf(" %s=%d [%s]\n", PARAMS_USE_DHCP, (int) m_tNetworkParams.bIsDhcpUsed, BOOL2STRING(m_tNetworkParams.bIsDhcpUsed));
	}

	if (isMaskSet(SET_IP_ADDRESS_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_IP_ADDRESS, IP2STR(m_tNetworkParams.nLocalIp));
	}

	if (isMaskSet(SET_NET_MASK_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_NET_MASK, IP2STR(m_tNetworkParams.nNetmask));
	}

	if (isMaskSet(SET_DEFAULT_GATEWAY_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_DEFAULT_GATEWAY, IP2STR(m_tNetworkParams.nGatewayIp));
	}

	if (isMaskSet(SET_NAME_SERVER_MASK)) {
		printf(" %s=" IPSTR "\n", PARAMS_NAME_SERVER, IP2STR(m_tNetworkParams.nNameServerIp));
	}

	if (isMaskSet(SET_HOSTNAME_MASK)) {
		printf(" %s=%s\n", PARAMS_HOSTNAME, m_tNetworkParams.aHostName);
	}
#endif
}

bool NetworkParams::isMaskSet(uint32_t nMask) const {
	return (m_tNetworkParams.bSetList & nMask) == nMask;
}

uint32_t NetworkParams::GetMaskIpAddress(void) {
	return SET_IP_ADDRESS_MASK;
}

uint32_t NetworkParams::GetMaskNetMask(void) {
	return SET_NET_MASK_MASK;
}

uint32_t NetworkParams::GetMaskDhcpUsed(void) {
	return SET_IS_DHCP_MASK;
}
