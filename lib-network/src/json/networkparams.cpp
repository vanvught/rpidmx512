/**
 * @file networkparams.cpp
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

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ip4/ip4_address.h"
#include "network.h"
#include "configstore.h"
#include "json/networkparams.h"
#include "json/networkparamsconst.h"
#include "json/json_parser.h"
#include "ip4/ip4_helpers.h"
#if defined(HAVE_NTP_CLIENT)
#include "apps/ntpclient.h"
#endif
#include "common/utils/utils_flags.h"

using common::store::network::Flags;

namespace json
{

NetworkParams::NetworkParams()
{
    ConfigStore::Instance().Copy(&store_network, &ConfigurationStore::network);
}

void NetworkParams::SetUseStaticIp(const char* val, uint32_t len)
{
    if (len == 1) store_network.flags = common::SetFlagValue(store_network.flags, Flags::Flag::kUseStaticIp, val[0] != '0');
}

void NetworkParams::SetIpAddress(const char* val, uint32_t len)
{
    store_network.local_ip = net::ParseIpString(val, len);
}

void NetworkParams::SetNetMask(const char* val, uint32_t len)
{
    store_network.netmask = net::ParseIpString(val, len);
}

void NetworkParams::SetDefaultGateway(const char* val, uint32_t len)
{
    store_network.gateway_ip = net::ParseIpString(val, len);
}

void NetworkParams::SetHostname(const char* val, uint32_t len)
{
    len = len > (common::store::network::kHostnameSize - 1) ? common::store::network::kHostnameSize - 1 : len;
    memcpy(reinterpret_cast<char*>(store_network.host_name), val, len);

    for (uint32_t i = len; i < common::store::network::kHostnameSize - 1; i++)
    {
        store_network.host_name[i] = '\0';
    }
}

void NetworkParams::SetNtpServer(const char* val, [[maybe_unused]] uint32_t len)
{
    store_network.ntp_server_ip = net::ParseIpString(val, len);
}

void NetworkParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kNetworkKeys);
    ConfigStore::Instance().Store(&store_network, &ConfigurationStore::network);

#ifndef NDEBUG
    Dump();
#endif
}

void NetworkParams::Set()
{
    if (strncmp(network::iface::HostName(), reinterpret_cast<char*>(store_network.host_name), common::store::network::kHostnameSize - 1) != 0)
    {
        network::iface::SetHostname(reinterpret_cast<char*>(store_network.host_name));
        // When default is set, copy the hostname back
        strncpy(reinterpret_cast<char*>(store_network.host_name), network::iface::HostName(), common::store::network::kHostnameSize - 1);
        store_network.host_name[common::store::network::kHostnameSize - 1] = 0;
    }

    const auto kUseStaticIp = common::IsFlagSet(store_network.flags, Flags::Flag::kUseStaticIp);

    if (kUseStaticIp)
    {
        network::SetGatewayIp(store_network.gateway_ip);
        network::SetNetmask(store_network.netmask);
        network::SetPrimaryIp(store_network.local_ip);
    }
    else
    {
        network::iface::EnableDhcp();
    }

#if defined(CONFIG_NET_ENABLE_NTP_CLIENT)
    network::apps::ntpclient::SetServerIp(store_network.ntp_server_ip);
#endif
#if defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
    network::apps::ntpclient::ptp::SetServerIp(store_network.ntp_server_ip);
#endif

#ifndef NDEBUG
    Dump();
#endif
}

void NetworkParams::Dump()
{
    const auto kUseStaticIp = common::IsFlagSet(store_network.flags, Flags::Flag::kUseStaticIp);

    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::NetworkParamsConst::kFileName);
    printf(" %s=%u [%s]\n", json::NetworkParamsConst::kUseStaticIp.name, static_cast<uint32_t>(kUseStaticIp), kUseStaticIp ? "Yes" : "No");
    printf(" %s=" IPSTR "\n", json::NetworkParamsConst::kIpAddress.name, IP2STR(store_network.local_ip));
    printf(" %s=" IPSTR "\n", json::NetworkParamsConst::kNetMask.name, IP2STR(store_network.netmask));
    printf(" %s=" IPSTR "\n", json::NetworkParamsConst::kDefaultGateway.name, IP2STR(store_network.gateway_ip));
    printf(" %s=%s\n", json::NetworkParamsConst::kHostname.name, store_network.host_name);
#if defined(HAVE_NTP_CLIENT)
    printf(" %s=" IPSTR "\n", json::NetworkParamsConst::kNtpServer.name, IP2STR(store_network.ntp_server_ip));
#endif
}

} // namespace json