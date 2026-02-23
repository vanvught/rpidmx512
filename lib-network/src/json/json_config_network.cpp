/**
 * @file json_config_network.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(CONFIG_NET_ENABLE_NTP_CLIENT) || defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
#define HAVE_NTP_CLIENT
#endif

#include <cstdint>

#include "network.h"
#include "json/networkparamsconst.h"
#include "json/networkparams.h"
#include "json/json_helpers.h"
#include "ip4/ip4_helpers.h"
#include "core/netif.h"
#if defined(HAVE_NTP_CLIENT)
#include "apps/ntpclient.h"
#endif

namespace json::config
{
uint32_t GetNetwork(char* buffer, uint32_t length)
{
#if defined(HAVE_NTP_CLIENT)
    uint32_t ntp_server_ip = 0;
#if defined(CONFIG_NET_ENABLE_NTP_CLIENT)
    ntp_server_ip = network::apps::ntpclient::GetServerIp();
#endif
#if defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
    ntp_server_ip = network::apps::ntpclient::ptp::GetServerIp();
#endif
#endif

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    char ip[net::kIpBufferSize];

	    doc[json::NetworkParamsConst::kSecondaryIp.name] = net::FormatIp(network::GetSecondaryIp(), ip);
	    doc[json::NetworkParamsConst::kUseStaticIp.name] = !network::iface::Dhcp() ? 1 : 0;
	    doc[json::NetworkParamsConst::kIpAddress.name] = net::FormatIp(network::GetPrimaryIp(), ip);
	    doc[json::NetworkParamsConst::kNetMask.name] = net::FormatIp(network::GetNetmask(), ip);
	    doc[json::NetworkParamsConst::kDefaultGateway.name] = net::FormatIp(network::GetGatewayIp(), ip);
	    doc[json::NetworkParamsConst::kHostname.name] =  network::iface::HostName();
#if defined(HAVE_NTP_CLIENT)    
	    doc[json::NetworkParamsConst::kNtpServer.name] = net::FormatIp(ntp_server_ip, ip);
#endif
	});
}

void SetNetwork(const char* buffer, uint32_t buffer_size)
{
    ::json::NetworkParams network_params;
    network_params.Store(buffer, buffer_size);
    network_params.Set();
}
} // namespace json::config
