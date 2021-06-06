/**
 * @file networkparams.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "propertiesbuilder.h"

#include "debug.h"

void NetworkParams::Builder(const struct TNetworkParams *ptNetworkParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptNetworkParams != nullptr) {
		memcpy(&m_tNetworkParams, ptNetworkParams, sizeof(struct TNetworkParams));
	} else {
		m_pNetworkParamsStore->Copy(&m_tNetworkParams);
	}

	PropertiesBuilder builder(NetworkParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(NetworkParamsMask::IP_ADDRESS)) {
		m_tNetworkParams.nLocalIp = Network::Get()->GetIp();
	}

	if (!isMaskSet(NetworkParamsMask::NET_MASK)) {
		m_tNetworkParams.nNetmask = Network::Get()->GetNetmask();
	}

	if (!isMaskSet(NetworkParamsMask::DEFAULT_GATEWAY)) {
		m_tNetworkParams.nGatewayIp = Network::Get()->GetGatewayIp();
	}

	if (!isMaskSet(NetworkParamsMask::HOSTNAME)) {
		strncpy(m_tNetworkParams.aHostName, Network::Get()->GetHostName(), NETWORK_HOSTNAME_SIZE - 1);
		m_tNetworkParams.aHostName[NETWORK_HOSTNAME_SIZE - 1] = '\0';
	}

	builder.Add(NetworkParamsConst::USE_DHCP, m_tNetworkParams.bIsDhcpUsed, isMaskSet(NetworkParamsMask::DHCP));
	builder.Add(NetworkParamsConst::DHCP_RETRY_TIME, m_tNetworkParams.nDhcpRetryTime, isMaskSet(NetworkParamsMask::DHCP_RETRY_TIME));

	builder.AddComment("Static IP");
	builder.AddIpAddress(NetworkParamsConst::IP_ADDRESS, m_tNetworkParams.nLocalIp, isMaskSet(NetworkParamsMask::IP_ADDRESS));
	builder.AddIpAddress(NetworkParamsConst::NET_MASK, m_tNetworkParams.nNetmask, isMaskSet(NetworkParamsMask::NET_MASK));
	builder.AddIpAddress(NetworkParamsConst::DEFAULT_GATEWAY, m_tNetworkParams.nGatewayIp, isMaskSet(NetworkParamsMask::DEFAULT_GATEWAY));
#if defined(ESP8266)
	builder.AddIpAddress(NetworkParamsConst::NAME_SERVER, m_tNetworkParams.nNameServerIp, isMaskSet(NetworkParamsMask::NAME_SERVER));
#endif
	builder.Add(NetworkParamsConst::HOSTNAME, m_tNetworkParams.aHostName, isMaskSet(NetworkParamsMask::HOSTNAME));

	builder.AddComment("NTP Server");
	builder.AddIpAddress(NetworkParamsConst::NTP_SERVER, m_tNetworkParams.nNtpServerIp, isMaskSet(NetworkParamsMask::NTP_SERVER));
	builder.Add(NetworkParamsConst::NTP_UTC_OFFSET, m_tNetworkParams.fNtpUtcOffset, isMaskSet(NetworkParamsMask::NTP_UTC_OFFSET));

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
