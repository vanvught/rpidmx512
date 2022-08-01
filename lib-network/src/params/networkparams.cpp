/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "network.h"
#include "networkparams.h"
#include "networkparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

using namespace networkparams;

NetworkParams::NetworkParams(NetworkParamsStore *pNetworkParamsStore): m_pNetworkParamsStore(pNetworkParamsStore) {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct networkparams::Params));
	m_Params.bIsDhcpUsed = defaults::IS_DHCP_USED;
	m_Params.nDhcpRetryTime = defaults::DHCP_RETRY_TIME;

	DEBUG_EXIT
}

bool NetworkParams::Load() {
	DEBUG_ENTRY
	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(NetworkParams::staticCallbackFunction, this);

	if (configfile.Read(NetworkParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pNetworkParamsStore != nullptr) {
			m_pNetworkParamsStore->Update(&m_Params);
		}
	} else
#endif
	if (m_pNetworkParamsStore != nullptr) {
		m_pNetworkParamsStore->Copy(&m_Params);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void NetworkParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pNetworkParamsStore != nullptr);

	if (m_pNetworkParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_Params.nSetList = 0;

	ReadConfigFile config(NetworkParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pNetworkParamsStore->Update(&m_Params);

	DEBUG_EXIT
}

void NetworkParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, NetworkParamsConst::USE_DHCP, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {	// Default
			m_Params.nSetList &= ~networkparams::Mask::DHCP;
		} else {
			m_Params.nSetList |= networkparams::Mask::DHCP;
		}
		m_Params.bIsDhcpUsed = !(nValue8 == 0);
		return;
	}

	if (Sscan::Uint8(pLine, NetworkParamsConst::DHCP_RETRY_TIME, nValue8) == Sscan::OK) {
		if ((nValue8 != defaults::DHCP_RETRY_TIME) && (nValue8 <= 5)) {
			m_Params.nSetList |= networkparams::Mask::DHCP_RETRY_TIME;
			m_Params.nDhcpRetryTime = nValue8;
		} else {
			m_Params.nSetList &= ~networkparams::Mask::DHCP_RETRY_TIME;
			m_Params.nDhcpRetryTime = defaults::DHCP_RETRY_TIME;
		}
	}

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, NetworkParamsConst::IP_ADDRESS, nValue32) == Sscan::OK) {
		if ((network::is_private_ip(nValue32)) || (nValue32 == 0)) {
			m_Params.nLocalIp = nValue32;
			m_Params.nSetList |= networkparams::Mask::IP_ADDRESS;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::NET_MASK, nValue32) == Sscan::OK) {
		if (network::is_netmask_valid(nValue32)) {
			m_Params.nNetmask = nValue32;
			m_Params.nSetList |= networkparams::Mask::NET_MASK;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::DEFAULT_GATEWAY, nValue32) == Sscan::OK) {
		m_Params.nGatewayIp = nValue32;
		m_Params.nSetList |= networkparams::Mask::DEFAULT_GATEWAY;
		return;
	}

	uint32_t nLength = network::HOSTNAME_SIZE - 1;

	if (Sscan::Char(pLine, NetworkParamsConst::HOSTNAME, m_Params.aHostName, nLength) == Sscan::OK) {
		m_Params.aHostName[nLength] = '\0';
		m_Params.nSetList |= networkparams::Mask::HOSTNAME;
		return;
	}


#if !defined(DISABLE_RTC)
	if (Sscan::IpAddress(pLine, NetworkParamsConst::NTP_SERVER, nValue32) == Sscan::OK) {
		if (nValue32 != 0) {
			m_Params.nSetList |= networkparams::Mask::NTP_SERVER;
		} else {
			m_Params.nSetList &= ~networkparams::Mask::NTP_SERVER;
		}
		m_Params.nNtpServerIp = nValue32;
		return;
	}

	float fValue;

	if (Sscan::Float(pLine, NetworkParamsConst::NTP_UTC_OFFSET, fValue) == Sscan::OK) {
		// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
		if ((static_cast<int32_t>(fValue) >= -12) && (static_cast<int32_t>(fValue) <= 14) && (fValue != defaults::NTP_UTC_OFFSET)) {
			m_Params.fNtpUtcOffset = fValue;
			m_Params.nSetList |= networkparams::Mask::NTP_UTC_OFFSET;
		} else {
			m_Params.fNtpUtcOffset = defaults::NTP_UTC_OFFSET;
			m_Params.nSetList &= ~networkparams::Mask::NTP_UTC_OFFSET;
		}
		return;
	}
#endif

#if defined (ESP8266)
	if (Sscan::IpAddress(pLine,  NetworkParamsConst::NAME_SERVER, nValue32) == Sscan::OK) {
		m_Params.nNameServerIp = nValue32;
		m_Params.nSetList |= networkparams::Mask::NAME_SERVER;
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::SSID, m_Params.aSsid, nLength) == Sscan::OK) {
		m_Params.aSsid[nLength] = '\0';
		m_Params.nSetList |= networkparams::Mask::SSID;
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::PASSWORD, m_Params.aPassword, nLength) == Sscan::OK) {
		m_Params.aPassword[nLength] = '\0';
		m_Params.nSetList |= networkparams::Mask::PASSWORD;
		return;
	}
#endif
}

void NetworkParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<NetworkParams*>(p))->callbackFunction(s);
}

void NetworkParams::Builder(const struct networkparams::Params *ptNetworkParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptNetworkParams != nullptr) {
		memcpy(&m_Params, ptNetworkParams, sizeof(struct networkparams::Params));
	} else {
		m_pNetworkParamsStore->Copy(&m_Params);
	}

	PropertiesBuilder builder(NetworkParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(networkparams::Mask::IP_ADDRESS)) {
		m_Params.nLocalIp = Network::Get()->GetIp();
	}

	if (!isMaskSet(networkparams::Mask::NET_MASK)) {
		m_Params.nNetmask = Network::Get()->GetNetmask();
	}

	if (!isMaskSet(networkparams::Mask::DEFAULT_GATEWAY)) {
		m_Params.nGatewayIp = Network::Get()->GetGatewayIp();
	}

	if (!isMaskSet(networkparams::Mask::HOSTNAME)) {
		strncpy(m_Params.aHostName, Network::Get()->GetHostName(), network::HOSTNAME_SIZE - 1);
		m_Params.aHostName[network::HOSTNAME_SIZE - 1] = '\0';
	}

	builder.Add(NetworkParamsConst::USE_DHCP, m_Params.bIsDhcpUsed, isMaskSet(networkparams::Mask::DHCP));
	builder.Add(NetworkParamsConst::DHCP_RETRY_TIME, m_Params.nDhcpRetryTime, isMaskSet(networkparams::Mask::DHCP_RETRY_TIME));

	builder.AddComment("Static IP");
	builder.AddIpAddress(NetworkParamsConst::IP_ADDRESS, m_Params.nLocalIp, isMaskSet(networkparams::Mask::IP_ADDRESS));
	builder.AddIpAddress(NetworkParamsConst::NET_MASK, m_Params.nNetmask, isMaskSet(networkparams::Mask::NET_MASK));
	builder.AddIpAddress(NetworkParamsConst::DEFAULT_GATEWAY, m_Params.nGatewayIp, isMaskSet(networkparams::Mask::DEFAULT_GATEWAY));
#if defined(ESP8266)
	builder.AddIpAddress(NetworkParamsConst::NAME_SERVER, m_Params.nNameServerIp, isMaskSet(networkparams::Mask::NAME_SERVER));
#endif
	builder.Add(NetworkParamsConst::HOSTNAME, m_Params.aHostName, isMaskSet(networkparams::Mask::HOSTNAME));

#if !defined(DISABLE_RTC)
	builder.AddComment("NTP Server");
	builder.AddIpAddress(NetworkParamsConst::NTP_SERVER, m_Params.nNtpServerIp, isMaskSet(networkparams::Mask::NTP_SERVER));
	builder.Add(NetworkParamsConst::NTP_UTC_OFFSET, m_Params.fNtpUtcOffset, isMaskSet(networkparams::Mask::NTP_UTC_OFFSET));
#endif

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void NetworkParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pNetworkParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
