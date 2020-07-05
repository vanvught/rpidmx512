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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "networkparams.h"
#include "network.h"
#include "networkconst.h"

#include "readconfigfile.h"
#include "sscan.h"

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	memset(&m_tNetworkParams, 0, sizeof(struct TNetworkParams));
	m_tNetworkParams.bIsDhcpUsed = true;
}

bool NetworkParams::Load() {
	m_tNetworkParams.nSetList = 0;

	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);

	if (configfile.Read(NetworkConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pNetworkParamsStore != nullptr) {
			m_pNetworkParamsStore->Update(&m_tNetworkParams);
		}
	} else if (m_pNetworkParamsStore != nullptr) {
		m_pNetworkParamsStore->Copy(&m_tNetworkParams);
	} else {
		return false;
	}

	return true;
}

void NetworkParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pNetworkParamsStore != nullptr);

	if (m_pNetworkParamsStore == nullptr) {
		return;
	}

	m_tNetworkParams.nSetList = 0;

	ReadConfigFile config(NetworkParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pNetworkParamsStore->Update(&m_tNetworkParams);
}

void NetworkParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint32_t nValue32;
	float f;

	if (Sscan::Uint8(pLine, NetworkConst::PARAMS_USE_DHCP, nValue8) == Sscan::OK) {
		m_tNetworkParams.bIsDhcpUsed = !(nValue8 == 0);
		m_tNetworkParams.nSetList |= NetworkParamsMask::DHCP;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_IP_ADDRESS, nValue32) == Sscan::OK) {
		m_tNetworkParams.nLocalIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::IP_ADDRESS;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_NET_MASK, nValue32) == Sscan::OK) {
		m_tNetworkParams.nNetmask = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::NET_MASK;
		return;
	}

	uint32_t nLength = NETWORK_HOSTNAME_SIZE - 1;
	if (Sscan::Char(pLine, NetworkConst::PARAMS_HOSTNAME, m_tNetworkParams.aHostName, nLength) == Sscan::OK) {
		m_tNetworkParams.aHostName[nLength] = '\0';
		m_tNetworkParams.nSetList |= NetworkParamsMask::HOSTNAME;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_NTP_SERVER, nValue32) == Sscan::OK) {
		m_tNetworkParams.nNtpServerIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::NTP_SERVER;
		return;
	}

	if (Sscan::Float(pLine, NetworkConst::PARAMS_NTP_UTC_OFFSET, f) == Sscan::OK) {
		// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
		if ((static_cast<int32_t>(f) >= -12) && (static_cast<int32_t>(f) <= 14)) {
			m_tNetworkParams.fNtpUtcOffset = f;
			m_tNetworkParams.nSetList |= NetworkParamsMask::NTP_UTC_OFFSET;
			return;
		}
	}

#if !defined (H3)
	if (Sscan::IpAddress(pLine, NetworkConst::PARAMS_DEFAULT_GATEWAY, nValue32) == Sscan::OK) {
		m_tNetworkParams.nGatewayIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::DEFAULT_GATEWAY;
		return;
	}

	if (Sscan::IpAddress(pLine,  NetworkConst::PARAMS_NAME_SERVER, nValue32) == Sscan::OK) {
		m_tNetworkParams.nNameServerIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::NAME_SERVER;
		return;
	}
#endif
}

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<NetworkParams*>(p))->callbackFunction(s);
}
