/**
 * @file network_iface.cpp
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

#include "network.h"
#include "net/ip4_address.h"
#include "net/dhcp.h"
#include "net/autoip.h"
#include "net_config.h"
#if !defined(CONFIG_NET_APPS_NO_MDNS)
#include "net/apps/mdns.h"
#endif
#include "network_store.h"
#include "network_display.h"
#include "firmware/debug/debug_debug.h"

namespace network::iface
{
static char s_hostname[net::HOSTNAME_SIZE];
static char s_domain_name[net::DOMAINNAME_SIZE];
static uint32_t s_nameservers[net::NAMESERVERS_COUNT];

static constexpr char ToHex(char i)
{
    return static_cast<char>(((i) < 10) ? '0' + i : 'A' + (i - 10));
}

void SetDomainName(const char* domainname)
{
    strncpy(s_domain_name, domainname, net::DOMAINNAME_SIZE - 1);
    s_domain_name[net::DOMAINNAME_SIZE - 1] = '\0';
}

const char* DomainName()
{
    return &s_domain_name[0];
}

void SetHostname(const char* hostname)
{
    DEBUG_ENTRY();

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    mdns::SendAnnouncement(0);
#endif

    if ((hostname == nullptr) || ((hostname != nullptr) && (hostname[0] == '\0')))
    {
        uint32_t k = 0;

        for (uint32_t i = 0; (i < (sizeof(HOST_NAME_PREFIX) - 1)) && (i < net::HOSTNAME_SIZE - 7); i++)
        {
            s_hostname[k++] = HOST_NAME_PREFIX[i];
        }

        const auto kHwaddr = netif::globals::netif_default.hwaddr;

        s_hostname[k++] = ToHex(kHwaddr[3] >> 4);
        s_hostname[k++] = ToHex(kHwaddr[3] & 0x0F);
        s_hostname[k++] = ToHex(kHwaddr[4] >> 4);
        s_hostname[k++] = ToHex(kHwaddr[4] & 0x0F);
        s_hostname[k++] = ToHex(kHwaddr[5] >> 4);
        s_hostname[k++] = ToHex(kHwaddr[5] & 0x0F);
        s_hostname[k] = '\0';
    }
    else
    {
        strncpy(s_hostname, hostname, net::HOSTNAME_SIZE - 1);
        s_hostname[net::HOSTNAME_SIZE - 1] = '\0';
    }

    network::store::SaveHostname(s_hostname, static_cast<uint32_t>(strlen(s_hostname)));
    netif::globals::netif_default.hostname = s_hostname;

#if !defined(CONFIG_NET_APPS_NO_MDNS)
    mdns::SendAnnouncement(mdns::kMdnsResponseTtl);
#endif
    network::display::Hostname();

    DEBUG_EXIT();
}

uint32_t NameServer(uint32_t index)
{
    if (index < net::NAMESERVERS_COUNT)
    {
        return s_nameservers[index];
    }

    return 0;
}

void EnableDhcp()
{
    DEBUG_ENTRY();

    net::dhcp::Start();

    network::store::SaveDhcp(true);

    DEBUG_EXIT();
}

void SetAutoIp()
{
    DEBUG_ENTRY();

    net::autoip::Start();

    network::store::SaveDhcp(false);

    DEBUG_EXIT();
}
} // namespace network::iface