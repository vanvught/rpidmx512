/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "networkparamsconst.h"
#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

using namespace networkparams;

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	memset(&m_tNetworkParams, 0, sizeof(struct TNetworkParams));
	m_tNetworkParams.bIsDhcpUsed = defaults::IS_DHCP_USED;
	m_tNetworkParams.nDhcpRetryTime = defaults::DHCP_RETRY_TIME;
}

bool NetworkParams::Load() {
	m_tNetworkParams.nSetList = 0;

	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);

	if (configfile.Read(NetworkParamsConst::FILE_NAME)) {
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

	if (Sscan::Uint8(pLine, NetworkParamsConst::USE_DHCP, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {	// Default
			m_tNetworkParams.nSetList &= ~NetworkParamsMask::DHCP;
		} else {
			m_tNetworkParams.nSetList |= NetworkParamsMask::DHCP;
		}
		m_tNetworkParams.bIsDhcpUsed = !(nValue8 == 0);
		return;
	}

	if (Sscan::Uint8(pLine, NetworkParamsConst::DHCP_RETRY_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 != defaults::DHCP_RETRY_TIME) && (nValue8 <= 5)) {
			m_tNetworkParams.nSetList |= NetworkParamsMask::DHCP_RETRY_TIME;
			m_tNetworkParams.nDhcpRetryTime = nValue8;
		} else {
			m_tNetworkParams.nSetList &= ~NetworkParamsMask::DHCP_RETRY_TIME;
			m_tNetworkParams.nDhcpRetryTime = defaults::DHCP_RETRY_TIME;
		}
	}

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, NetworkParamsConst::IP_ADDRESS, nValue32) == Sscan::OK) {
		m_tNetworkParams.nLocalIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::IP_ADDRESS;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::NET_MASK, nValue32) == Sscan::OK) {
		m_tNetworkParams.nNetmask = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::NET_MASK;
		return;
	}

	uint32_t nLength = NETWORK_HOSTNAME_SIZE - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::HOSTNAME, m_tNetworkParams.aHostName, nLength) == Sscan::OK) {
		m_tNetworkParams.aHostName[nLength] = '\0';
		m_tNetworkParams.nSetList |= NetworkParamsMask::HOSTNAME;
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::NTP_SERVER, nValue32) == Sscan::OK) {
		if (nValue32 != 0) {
			m_tNetworkParams.nSetList |= NetworkParamsMask::NTP_SERVER;
		} else {
			m_tNetworkParams.nSetList &= ~NetworkParamsMask::NTP_SERVER;
		}
		m_tNetworkParams.nNtpServerIp = nValue32;
		return;
	}

	float fValue;

	if (Sscan::Float(pLine, NetworkParamsConst::NTP_UTC_OFFSET, fValue) == Sscan::OK) {
		// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
		if ((static_cast<int32_t>(fValue) >= -12) && (static_cast<int32_t>(fValue) <= 14) && (fValue != defaults::NTP_UTC_OFFSET)) {
			m_tNetworkParams.fNtpUtcOffset = fValue;
			m_tNetworkParams.nSetList |= NetworkParamsMask::NTP_UTC_OFFSET;
		} else {
			m_tNetworkParams.fNtpUtcOffset = defaults::NTP_UTC_OFFSET;
			m_tNetworkParams.nSetList &= ~NetworkParamsMask::NTP_UTC_OFFSET;
		}
		return;
	}

#if defined (ESP8266)
	if (Sscan::IpAddress(pLine, NetworkParamsConst::DEFAULT_GATEWAY, nValue32) == Sscan::OK) {
		m_tNetworkParams.nGatewayIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::DEFAULT_GATEWAY;
		return;
	}

	if (Sscan::IpAddress(pLine,  NetworkParamsConst::NAME_SERVER, nValue32) == Sscan::OK) {
		m_tNetworkParams.nNameServerIp = nValue32;
		m_tNetworkParams.nSetList |= NetworkParamsMask::NAME_SERVER;
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::SSID, m_tNetworkParams.aSsid, nLength) == Sscan::OK) {
		m_tNetworkParams.aSsid[nLength] = '\0';
		m_tNetworkParams.nSetList |= NetworkParamsMask::SSID;
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::PASSWORD, m_tNetworkParams.aPassword, nLength) == Sscan::OK) {
		m_tNetworkParams.aPassword[nLength] = '\0';
		m_tNetworkParams.nSetList |= NetworkParamsMask::PASSWORD;
		return;
	}
#endif
}

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<NetworkParams*>(p))->callbackFunction(s);
}
