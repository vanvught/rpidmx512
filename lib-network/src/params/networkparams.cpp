/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifdef DEBUG_NETWORK
# undef NDEBUG
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

namespace networkparams::store {
	static void Update(const struct networkparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::NETWORK, pParams, sizeof(struct networkparams::Params));
	}

	static void Copy(struct networkparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::NETWORK, pParams, sizeof(struct networkparams::Params));
	}
}  // namespace networkparams::store

NetworkParams::NetworkParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void NetworkParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(NetworkParams::StaticCallbackFunction, this);

	if (configfile.Read(NetworkParamsConst::FILE_NAME)) {
		networkparams::store::Update(&m_Params);
	} else
#endif
		networkparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void NetworkParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(m_Params));

	ReadConfigFile config(NetworkParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	networkparams::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void NetworkParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, NetworkParamsConst::USE_STATIC_IP, nValue8) == Sscan::OK) {
		m_Params.bUseStaticIp = (nValue8 != 0);
		return;
	}

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, NetworkParamsConst::IP_ADDRESS, nValue32) == Sscan::OK) {
		if ((net::is_private_ip(nValue32)) || ((nValue32 & 0xFF) == 2U) || (nValue32 == 0)) {
			m_Params.nLocalIp = nValue32;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::NET_MASK, nValue32) == Sscan::OK) {
		if (net::is_netmask_valid(nValue32)) {
			m_Params.nNetmask = nValue32;
		}
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::DEFAULT_GATEWAY, nValue32) == Sscan::OK) {
		if (nValue32 != 0) {
			m_Params.nGatewayIp = nValue32;
		}

		return;
	}

	uint32_t nLength = net::HOSTNAME_SIZE - 1;

	if (Sscan::Char(pLine, NetworkParamsConst::HOSTNAME, m_Params.aHostName, nLength) == Sscan::OK) {
		m_Params.aHostName[nLength] = '\0';
		return;
	}

	if (Sscan::IpAddress(pLine, NetworkParamsConst::NTP_SERVER, nValue32) == Sscan::OK) {
		m_Params.nNtpServerIp = nValue32;
		if (nValue32 != 0) {
			m_Params.nSetList |= networkparams::Mask::NTP_SERVER;
		} else {
			m_Params.nSetList &= ~networkparams::Mask::NTP_SERVER;
		}
		return;
	}

#if defined (ESP8266)
	if (Sscan::IpAddress(pLine,  NetworkParamsConst::NAME_SERVER, nValue32) == Sscan::OK) {
		m_Params.nNameServerIp = nValue32;
		m_Params.nSetList |= networkparams::Mask::NAME_SERVER;
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::SSID, m_Params.aSsid, nLength) == Sscan::OK) {
		m_Params.aSsid[nLength] = '\0';
		return;
	}

	nLength = 34 - 1;
	if (Sscan::Char(pLine, NetworkParamsConst::PASSWORD, m_Params.aPassword, nLength) == Sscan::OK) {
		m_Params.aPassword[nLength] = '\0';
		return;
	}
#endif
}

void NetworkParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<NetworkParams*>(p))->CallbackFunction(s);
}

void NetworkParams::Builder(const struct networkparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("pParams=%p", pParams);

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct networkparams::Params));
	} else {
		networkparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(NetworkParamsConst::FILE_NAME, pBuffer, nLength);

	// Fixed
	builder.AddIpAddress("secondary_ip", Network::Get()->GetSecondaryIp(), false);
	builder.Add(NetworkParamsConst::USE_STATIC_IP, m_Params.bUseStaticIp, true);

	builder.AddComment("Static IP");

	if (m_Params.nLocalIp == 0) {
		m_Params.nLocalIp = Network::Get()->GetIp();
	}

	builder.AddIpAddress(NetworkParamsConst::IP_ADDRESS, m_Params.nLocalIp);

	if (m_Params.nNetmask == 0) {
		m_Params.nNetmask = Network::Get()->GetNetmask();
	}

	builder.AddIpAddress(NetworkParamsConst::NET_MASK, m_Params.nNetmask);


	if (m_Params.nGatewayIp == 0) {
		m_Params.nGatewayIp = Network::Get()->GetGatewayIp();
	}

	builder.AddIpAddress(NetworkParamsConst::DEFAULT_GATEWAY, m_Params.nGatewayIp);
#if defined(ESP8266)
	builder.AddIpAddress(NetworkParamsConst::NAME_SERVER, m_Params.nNameServerIp);
#endif

	const auto isHostNameSet = (m_Params.aHostName[0] != '\0');

	if (!isHostNameSet) {
		strncpy(m_Params.aHostName, Network::Get()->GetHostName(), net::HOSTNAME_SIZE - 1);
		m_Params.aHostName[net::HOSTNAME_SIZE - 1] = '\0';
	}

	builder.Add(NetworkParamsConst::HOSTNAME, m_Params.aHostName, isHostNameSet);

	builder.AddComment("NTP Server");
	builder.AddIpAddress(NetworkParamsConst::NTP_SERVER, m_Params.nNtpServerIp, IsMaskSet(networkparams::Mask::NTP_SERVER));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void NetworkParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NetworkParamsConst::FILE_NAME);

	printf(" %s=%d [%s]\n", NetworkParamsConst::USE_STATIC_IP, m_Params.bUseStaticIp, m_Params.bUseStaticIp ? "Yes" : "No");
	printf(" %s=" IPSTR "\n", NetworkParamsConst::IP_ADDRESS, IP2STR(m_Params.nLocalIp));
	printf(" %s=" IPSTR "\n", NetworkParamsConst::NET_MASK, IP2STR(m_Params.nNetmask));
	printf(" %s=" IPSTR "\n", NetworkParamsConst::DEFAULT_GATEWAY, IP2STR(m_Params.nGatewayIp));

#if defined (ESP8266)
	printf(" %s=" IPSTR "\n",  NetworkParamsConst::NAME_SERVER, IP2STR(m_Params.nNameServerIp));
#endif

	printf(" %s=%s\n", NetworkParamsConst::HOSTNAME, m_Params.aHostName);
	printf(" %s=" IPSTR "\n", NetworkParamsConst::NTP_SERVER, IP2STR(m_Params.nNtpServerIp));
}
