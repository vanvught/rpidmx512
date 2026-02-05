/**
 * @file network_iface.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "linux/network.h"
#include "network_store.h"
#include "firmware/debug/debug_debug.h"

char s_hostname[network::iface::kHostnameSize];
char s_domain_name[network::iface::kDomainnameSize];
uint32_t s_nameservers[network::iface::kNameserversCount];

namespace network::iface
{
bool Dhcp()
{
    return (netif::global::netif_default.flags & netif::Netif::kNetifFlagDhcpOk) == netif::Netif::kNetifFlagDhcpOk;
}

void EnableDhcp()
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

void SetAutoIp()
{
    DEBUG_ENTRY();
    DEBUG_EXIT();
}

bool AutoIp()
{
    return (netif::global::netif_default.flags & netif::Netif::kNetifFlagAutoipOk) == netif::Netif::kNetifFlagAutoipOk;
}

void SetHostname(const char* hostname)
{
    DEBUG_ENTRY();

    if ((hostname == nullptr) || ((hostname != nullptr) && (hostname[0] == '\0')))
    {
        return;
    }
    else
    {
        strncpy(s_hostname, hostname, network::iface::kHostnameSize - 1);
        s_hostname[network::iface::kHostnameSize - 1] = '\0';
    }

    network::store::SaveHostname(s_hostname, static_cast<uint32_t>(strlen(s_hostname)));

    netif::global::netif_default.hostname = s_hostname;

    DEBUG_EXIT();
}

void SetDomainName(const char* domainname)
{
    strncpy(s_domain_name, domainname, network::iface::kDomainnameSize - 1);
    s_domain_name[network::iface::kDomainnameSize - 1] = '\0';
}

const char* HostName()
{
    return netif::global::netif_default.hostname;
}

const char* DomainName()
{
    return &s_domain_name[0];
}

uint32_t NameServer(uint32_t index)
{
    if (index < network::iface::kNameserversCount)
    {
        return s_nameservers[index];
    }

    return 0;
}

uint32_t NameServerCount()
{
    return kNameserversCount;
}

} // namespace network::iface