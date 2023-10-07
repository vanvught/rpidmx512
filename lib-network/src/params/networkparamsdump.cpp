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

#include <cstdio>

#include "network.h"
#include "networkparams.h"
#include "networkparamsconst.h"

#include "debug.h"

void NetworkParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NetworkParamsConst::FILE_NAME);

	debug_print_bits(m_Params.nSetList);

	printf(" %s=%d [%s]\n", NetworkParamsConst::USE_DHCP, static_cast<int>(m_Params.bIsDhcpUsed), m_Params.bIsDhcpUsed != 0 ? "Yes" : "No");
	printf(" %s=" IPSTR "\n", NetworkParamsConst::IP_ADDRESS, IP2STR(m_Params.nLocalIp));
	printf(" %s=" IPSTR "\n", NetworkParamsConst::NET_MASK, IP2STR(m_Params.nNetmask));

#if defined (ESP8266)
	printf(" %s=" IPSTR "\n", NetworkParamsConst::DEFAULT_GATEWAY, IP2STR(m_Params.nGatewayIp));
	printf(" %s=" IPSTR "\n",  NetworkParamsConst::NAME_SERVER, IP2STR(m_Params.nNameServerIp));
#endif

	printf(" %s=%s\n", NetworkParamsConst::HOSTNAME, m_Params.aHostName);
	printf(" %s=" IPSTR "\n", NetworkParamsConst::NTP_SERVER, IP2STR(m_Params.nNtpServerIp));
	printf(" %s=%1.1f\n", NetworkParamsConst::NTP_UTC_OFFSET, m_Params.fNtpUtcOffset);
#endif
}
