/**
 * @file network_store.cpp
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <algorithm>

#include "common/utils/utils_flags.h"
#include "configstore.h"
#include "configurationstore.h"
#include "network_iface.h"

using common::store::network::Flags;

namespace network::store
{
__attribute__((weak)) void SaveIp(uint32_t ip)
{
    ConfigStore::Instance().NetworkUpdate(&common::store::Network::local_ip, ip);
}

__attribute__((weak)) void SaveNetmask(uint32_t netmask)
{
    ConfigStore::Instance().NetworkUpdate(&common::store::Network::netmask, netmask);
}

__attribute__((weak)) void SaveGatewayIp(uint32_t gateway_ip)
{
    ConfigStore::Instance().NetworkUpdate(&common::store::Network::gateway_ip, gateway_ip);
}

__attribute__((weak)) void SaveHostname(const char* hostname, uint32_t length)
{
    length = std::min(length, static_cast<uint32_t>(network::iface::kHostnameSize));
    ConfigStore::Instance().NetworkUpdateArray(&common::store::Network::host_name, hostname, length);
}

__attribute__((weak)) void SaveDhcp(bool is_dhcp_used)
{
    const auto kUseStaticIp = !is_dhcp_used;
    auto flags = ConfigStore::Instance().NetworkGet(&common::store::Network::flags);
    flags = common::SetFlagValue(flags, Flags::Flag::kUseStaticIp, kUseStaticIp);

    ConfigStore::Instance().NetworkUpdate(&common::store::Network::flags, flags);
}
} // namespace network::store