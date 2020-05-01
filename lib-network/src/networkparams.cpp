/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

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

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	memset(&m_tNetworkParams, 0, sizeof(struct TNetworkParams));
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

	uint8_t nValue8;
	uint32_t nValue32;
	uint8_t nLength;
	float f;

	if (Sscan::Uint8(pLine, NetworkConst::PARAMS_USE_DHCP, &nValue8) == SSCAN_OK) {
		m_tNetworkParams.bIsDhcpUsed = !(nValue8 == 0);
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_DHCP;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_IP_ADDRESS, &nValue32) == SSCAN_OK) {
		m_tNetworkParams.nLocalIp = nValue32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_IP_ADDRESS;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_NET_MASK, &nValue32) == SSCAN_OK) {
		m_tNetworkParams.nNetmask = nValue32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NET_MASK;
		return;
	}

	nLength = NETWORK_HOSTNAME_SIZE - 1;
	if (Sscan::Char(pLine, NetworkConst::PARAMS_HOSTNAME, m_tNetworkParams.aHostName, &nLength) == SSCAN_OK) {
		m_tNetworkParams.aHostName[nLength] = '\0';
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_HOSTNAME;
		return;
	}

#if !defined (H3)
	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_DEFAULT_GATEWAY, &nValue32) == SSCAN_OK) {
		m_tNetworkParams.nGatewayIp = nValue32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_DEFAULT_GATEWAY;
		return;
	}

	if (Sscan::IpAddress(pLine,  NetworkConst::PARAMS_NAME_SERVER, &nValue32) == SSCAN_OK) {
		m_tNetworkParams.nNameServerIp = nValue32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NAME_SERVER;
		return;
	}
#endif

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_NTP_SERVER, &nValue32) == SSCAN_OK) {
		m_tNetworkParams.nNtpServerIp = nValue32;
		m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NTP_SERVER;
		return;
	}

	if (Sscan::Float(pLine, NetworkConst::PARAMS_NTP_UTC_OFFSET, &f) == SSCAN_OK) {
		// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
		if ((static_cast<int32_t>(f) >= -12) && (static_cast<int32_t>(f) <= 14)) {
			m_tNetworkParams.fNtpUtcOffset = f;
			m_tNetworkParams.nSetList |= NETWORK_PARAMS_MASK_NTP_UTC_OFFSET;
			return;
		}
	}
}

void NetworkParams::Dump(void) {
#ifndef NDEBUG
	if (m_tNetworkParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NetworkConst::PARAMS_FILE_NAME);

	if (isMaskSet(NETWORK_PARAMS_MASK_DHCP)) {
		printf(" %s=%d [%s]\n", NetworkConst::PARAMS_USE_DHCP,
				static_cast<int>(m_tNetworkParams.bIsDhcpUsed),
				BOOL2STRING(m_tNetworkParams.bIsDhcpUsed));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_IP_ADDRESS)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_IP_ADDRESS, IP2STR(m_tNetworkParams.nLocalIp));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_NET_MASK)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_NET_MASK, IP2STR(m_tNetworkParams.nNetmask));
	}

#if !defined (H3)
	if (isMaskSet(NETWORK_PARAMS_MASK_DEFAULT_GATEWAY)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_DEFAULT_GATEWAY, IP2STR(m_tNetworkParams.nGatewayIp));
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_NAME_SERVER)) {
		printf(" %s=" IPSTR "\n",  NetworkConst::PARAMS_NAME_SERVER, IP2STR(m_tNetworkParams.nNameServerIp));
	}
#endif

	if (isMaskSet(NETWORK_PARAMS_MASK_HOSTNAME)) {
		printf(" %s=%s\n", NetworkConst::PARAMS_HOSTNAME, m_tNetworkParams.aHostName);
	}

	if (isMaskSet(NETWORK_PARAMS_MASK_NTP_SERVER)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_NTP_SERVER, IP2STR(m_tNetworkParams.nNtpServerIp));
	}
#endif
}

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<NetworkParams*>(p))->callbackFunction(s);
}

