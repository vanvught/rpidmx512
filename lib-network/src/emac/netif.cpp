/**
 * @file netif.cpp
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

#if defined (DEBUG_NETIF)
# undef NDEBUG
#endif

#include <cstring>

#include "net/netif.h"
#include "net/ip4_address.h"
#include "network_store.h"
#include "network_display.h"
#if !defined(CONFIG_NET_APPS_NO_MDNS)
# include "net/apps/mdns.h"
#endif

#include "../../config/net_config.h"

#include "debug.h"

namespace net {
static char s_hostname[net::HOSTNAME_SIZE];
static char s_domainName[net::DOMAINNAME_SIZE];
static uint32_t s_nameservers[net::NAMESERVERS_COUNT];

static constexpr char TO_HEX(const char i) {
	return static_cast<char>(((i) < 10) ? '0' + i : 'A' + (i - 10));
}

const char *netif_get_ifname() {
	return "eth0";
}

uint32_t netif_get_nameserver(const uint32_t nIndex) {
	if (nIndex < net::NAMESERVERS_COUNT) {
		return s_nameservers[nIndex];
	}

	return 0;
}

void netif_set_domainname(const char *pDomainname) {
	strncpy(s_domainName, pDomainname, net::DOMAINNAME_SIZE - 1);
	s_domainName[net::DOMAINNAME_SIZE - 1] = '\0';
}

const char *netif_get_domainname() {
	return &s_domainName[0];
}

void netif_set_hostname(const char *pHostname) {
	DEBUG_ENTRY

	if ((pHostname == nullptr) || ((pHostname != nullptr) && (pHostname[0] == '\0'))) {
		uint32_t k = 0;

		for (uint32_t i = 0; (i < (sizeof(HOST_NAME_PREFIX) - 1)) && (i < net::HOSTNAME_SIZE - 7); i++) {
			s_hostname[k++] = HOST_NAME_PREFIX[i];
		}

		const auto hwaddr = net::globals::netif_default.hwaddr;

		s_hostname[k++] = TO_HEX(hwaddr[3] >> 4);
		s_hostname[k++] = TO_HEX(hwaddr[3] & 0x0F);
		s_hostname[k++] = TO_HEX(hwaddr[4] >> 4);
		s_hostname[k++] = TO_HEX(hwaddr[4] & 0x0F);
		s_hostname[k++] = TO_HEX(hwaddr[5] >> 4);
		s_hostname[k++] = TO_HEX(hwaddr[5] & 0x0F);
		s_hostname[k] = '\0';
	} else {
		strncpy(s_hostname, pHostname, net::HOSTNAME_SIZE - 1);
		s_hostname[net::HOSTNAME_SIZE - 1] = '\0';
	}

	network_store_save_hostname(s_hostname, static_cast<uint32_t>(strlen(s_hostname)));

#if !defined(CONFIG_NET_APPS_NO_MDNS)
	mdns_send_announcement(mdns::MDNS_RESPONSE_TTL);
#endif
	network_display_hostname();

	net::globals::netif_default.hostname = s_hostname;

	DEBUG_EXIT
}
}  // namespace net
