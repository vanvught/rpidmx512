/**
 * @file autoip.cpp
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
/* This code is inspired by the lwIP TCP/IP stack.
 * https://savannah.nongnu.org/projects/lwip/
 */
/**
 * The autoip.cpp aims to be conform to RFC 3927.
 * https://datatracker.ietf.org/doc/html/rfc3927
 * Dynamic Configuration of IPv4 Link-Local Addresses
 */

#ifdef DEBUG_AUTOIP
# undef NDEBUG
#endif

#include <cstring>
#include <cassert>

#include "netif.h"
#include "net/autoip.h"
#include "net/protocol/autoip.h"
#include "net/acd.h"
#include "net/arp.h"

#include "debug.h"

namespace net {
static void autoip_bind(){
	auto *autoip = reinterpret_cast<struct autoip::Autoip *>(globals::netif_default.autoip);
	assert(autoip != nullptr);

	autoip->state = autoip::State::AUTOIP_STATE_BOUND;

	ip4_addr_t sn_mask, gw_addr;

	sn_mask.addr = network::convert_to_uint(255,255,0,0);
	gw_addr.addr = 0;

	netif_set_addr(autoip->llipaddr, sn_mask, gw_addr);
}

static void autoip_restart() {
	auto *autoip = reinterpret_cast<struct autoip::Autoip *>(globals::netif_default.autoip);
	assert(autoip != nullptr);

	autoip->tried_llipaddr++;
	autoip_start();
}

static void autoip_conflict_callback(net::acd::Callback state) {
	auto *autoip = reinterpret_cast<struct autoip::Autoip *>(globals::netif_default.autoip);
	assert(autoip != nullptr);

	switch (state) {
	case net::acd::Callback::ACD_IP_OK:
		autoip_bind();
		netif_set_flags(netif::NETIF_FLAG_AUTOIP_OK);
		break;
	case net::acd::Callback::ACD_RESTART_CLIENT:
		autoip_restart();
		break;
	case net::acd::Callback::ACD_DECLINE:
		/* "delete" conflicting address so a new one will be selected in autoip_start() */
		autoip->llipaddr.addr = network::IP4_ANY;
		autoip_stop();
		netif_clear_flags(netif::NETIF_FLAG_AUTOIP_OK);
		break;
	default:
		break;
	}
}

static void autoip_create_addr(uint32_t &ipaddr) {
	auto *autoip = reinterpret_cast<struct autoip::Autoip*>(globals::netif_default.autoip);
	assert(autoip != nullptr);

	/* Here we create an IP-Address out of range 169.254.1.0 to 169.254.254.255
	 * compliant to RFC 3927 Section 2.1 */
	const auto mask = globals::netif_default.hwaddr[3] + (globals::netif_default.hwaddr[4] << 8);
	ipaddr = static_cast<uint32_t>(mask << 16) | autoip::AUTOIP_RANGE_START;
	ipaddr = __builtin_bswap32(ipaddr);

	ipaddr += autoip->tried_llipaddr;
	ipaddr = __builtin_bswap32(autoip::AUTOIP_NET) | (ipaddr & 0xffff);

	if (ipaddr < __builtin_bswap32(autoip::AUTOIP_RANGE_START)) {
		ipaddr += __builtin_bswap32(autoip::AUTOIP_RANGE_END) - __builtin_bswap32(autoip::AUTOIP_RANGE_START) + 1;
	}

	if (ipaddr > __builtin_bswap32(autoip::AUTOIP_RANGE_END)) {
		ipaddr -= __builtin_bswap32(autoip::AUTOIP_RANGE_END) - __builtin_bswap32(autoip::AUTOIP_RANGE_START) + 1;
	}

	ipaddr = __builtin_bswap32(ipaddr);

	DEBUG_PRINTF(IPSTR, IP2STR(ipaddr));
}

/*
 * Public interface
 */

void autoip_start() {
	DEBUG_ENTRY

	auto *autoip = reinterpret_cast<struct autoip::Autoip *>(globals::netif_default.autoip);

	if (autoip == nullptr) {
		autoip = new (struct autoip::Autoip);
		assert(autoip != nullptr);
		memset(autoip, 0, sizeof(struct autoip::Autoip));
	}

	if (autoip->state == autoip::State::AUTOIP_STATE_OFF) {
		acd_add(&autoip->acd, autoip_conflict_callback);

		/* In accordance to RFC3927 section 2.1:
		 * Keep using the same link local address as much as possible.
		 * Only when there is none or when there was a conflict, select a new one.
		 */
		if (!network::is_linklocal_ip(autoip->llipaddr.addr)) {
			autoip_create_addr(autoip->llipaddr.addr);
		}

		autoip->state = autoip::State::AUTOIP_STATE_CHECKING;
		acd_start(&autoip->acd, autoip->llipaddr);
	} else {
		DEBUG_PUTS("Already started");
	}

	DEBUG_EXIT
}

void autoip_stop() {
	DEBUG_ENTRY
	auto *autoip = reinterpret_cast<struct autoip::Autoip *>(globals::netif_default.autoip);

	if (autoip != nullptr) {
		autoip->state = autoip::State::AUTOIP_STATE_OFF;

		ip4_addr_t any;
		any.addr = network::IP4_ANY;

		if (network::is_linklocal_ip(globals::netif_default.ip.addr)) {
			netif_set_addr(any, any, any);
		}
	}

	DEBUG_EXIT
}
}  // namespace net
