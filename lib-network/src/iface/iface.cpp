/**
 * @file iface.cpp
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

#include <cstring>
#include <cassert>

#include "network.h"
#include "core/ip4/dhcp.h"
#include "core/ip4/autoip.h"
#include "net_config.h"
#include "network_iface.h"
#if !defined(CONFIG_NET_APPS_NO_MDNS)
#include "apps/mdns.h"
#endif
#include "network_store.h"
#include "network_display.h"
#include "firmware/debug/debug_debug.h"

namespace network::iface
{
static char s_hostname[kHostnameSize];
static char s_domain_name[kDomainnameSize];
static uint32_t s_nameservers[kNameserversCount];

static constexpr char ToHex(char i)
{
    return static_cast<char>(((i) < 10) ? '0' + i : 'A' + (i - 10));
}

void CopyMacAddressTo(uint8_t* mac_address)
{
	assert(mac_address != nullptr);
    memcpy(mac_address, netif::HwAddr(), kMacSize);
}

void SetDomainName(const char* domainname)
{
    if (domainname == nullptr || domainname[0] == '\0')
    {
        s_domain_name[0] = '\0';
        return;
    }

    strncpy(s_domain_name, domainname, kDomainnameSize - 1);
    s_domain_name[kDomainnameSize - 1] = '\0';
}

const char* DomainName()
{
    return &s_domain_name[0];
}

static void BuildDefaultHostname()
{
    uint32_t k = 0;

    static constexpr uint32_t kSuffixLen = 6;            // 3 bytes -> 6 hex chars
    static constexpr uint32_t kMinTail = kSuffixLen + 1; // + '\0'

    const uint32_t kMaxPrefix = (kHostnameSize > kMinTail) ? (kHostnameSize - kMinTail) : 0;

    for (uint32_t i = 0; i < (sizeof(HOST_NAME_PREFIX) - 1) && i < kMaxPrefix; ++i)
    {
        s_hostname[k++] = HOST_NAME_PREFIX[i];
    }

    const auto& hw = netif::global::netif_default.hwaddr; // expects at least 6 bytes

    s_hostname[k++] = ToHex(hw[3] >> 4);
    s_hostname[k++] = ToHex(hw[3]);
    s_hostname[k++] = ToHex(hw[4] >> 4);
    s_hostname[k++] = ToHex(hw[4]);
    s_hostname[k++] = ToHex(hw[5] >> 4);
    s_hostname[k++] = ToHex(hw[5]);

    s_hostname[k] = '\0';
}

void SetHostnameAuto()
{
    BuildDefaultHostname();
}

void SetHostname(const char* hostname)
{
    DEBUG_ENTRY();

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    network::apps::mdns::SendAnnouncement(0);
#endif

    if (hostname == nullptr || hostname[0] == '\0')
    {
        BuildDefaultHostname();
    }
    else
    {
        strncpy(s_hostname, hostname, kHostnameSize - 1);
        s_hostname[kHostnameSize - 1] = '\0';
    }

    const auto kLength = static_cast<uint32_t>(strlen(s_hostname));
    network::store::SaveHostname(s_hostname, kLength);

    netif::global::netif_default.hostname = s_hostname;

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    network::apps::mdns::SendAnnouncement(network::apps::mdns::kMdnsResponseTtl);
#endif
    network::display::Hostname();

    DEBUG_EXIT();
}

const char* HostName()
{
    return netif::global::netif_default.hostname;
}

uint32_t NameServer(uint32_t index)
{
    if (index < kNameserversCount)
    {
        return s_nameservers[index];
    }

    return 0;
}

uint32_t NameServerCount()
{
    return kNameserversCount;
}

bool Dhcp()
{
    return (netif::global::netif_default.flags & netif::Netif::kNetifFlagDhcpOk) == netif::Netif::kNetifFlagDhcpOk;
}

void EnableDhcp()
{
    DEBUG_ENTRY();

    network::dhcp::Start();

    network::store::SaveDhcp(true);

    DEBUG_EXIT();
}

void SetAutoIp()
{
    DEBUG_ENTRY();

    network::autoip::Start();

    network::store::SaveDhcp(false);

    DEBUG_EXIT();
}

bool AutoIp()
{
    return (netif::global::netif_default.flags & netif::Netif::kNetifFlagAutoipOk) == netif::Netif::kNetifFlagAutoipOk;
}
} // namespace network::iface