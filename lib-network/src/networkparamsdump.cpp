/**
 * @file networkparamsdump.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>

#include "networkparams.h"
#include "network.h"
#include "networkconst.h"

#include "debug.h"

void NetworkParams::Dump() {
#ifndef NDEBUG
	if (m_tNetworkParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NetworkConst::PARAMS_FILE_NAME);

	if (isMaskSet(NetworkParamsMask::DHCP)) {
		printf(" %s=%d [%s]\n", NetworkConst::PARAMS_USE_DHCP, static_cast<int>(m_tNetworkParams.bIsDhcpUsed), BOOL2STRING::Get(m_tNetworkParams.bIsDhcpUsed));
	}

	if (isMaskSet(NetworkParamsMask::IP_ADDRESS)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_IP_ADDRESS, IP2STR(m_tNetworkParams.nLocalIp));
	}

	if (isMaskSet(NetworkParamsMask::NET_MASK)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_NET_MASK, IP2STR(m_tNetworkParams.nNetmask));
	}

#if !defined (H3)
	if (isMaskSet(NetworkParamsMask::DEFAULT_GATEWAY)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_DEFAULT_GATEWAY, IP2STR(m_tNetworkParams.nGatewayIp));
	}

	if (isMaskSet(NetworkParamsMask::NAME_SERVER)) {
		printf(" %s=" IPSTR "\n",  NetworkConst::PARAMS_NAME_SERVER, IP2STR(m_tNetworkParams.nNameServerIp));
	}
#endif

	if (isMaskSet(NetworkParamsMask::HOSTNAME)) {
		printf(" %s=%s\n", NetworkConst::PARAMS_HOSTNAME, m_tNetworkParams.aHostName);
	}

	if (isMaskSet(NetworkParamsMask::NTP_SERVER)) {
		printf(" %s=" IPSTR "\n", NetworkConst::PARAMS_NTP_SERVER, IP2STR(m_tNetworkParams.nNtpServerIp));
	}

	if (isMaskSet(NetworkParamsMask::NTP_UTC_OFFSET)) {
		printf(" %s=%1.1f\n", NetworkConst::PARAMS_NTP_UTC_OFFSET, m_tNetworkParams.fNtpUtcOffset);
	}
#endif
}
