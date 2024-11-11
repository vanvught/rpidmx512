/**
 * @file netif.cpp
 *
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

#if defined (DEBUG_NET_NETIF)
# undef NDEBUG
#endif

#include <cstring>

#include "netif.h"
#include "net/acd.h"
#include "net/autoip.h"
#include "net/dhcp.h"
#include "net/igmp.h"

#include "debug.h"

namespace net {
namespace globals {
struct netif netif_default;
extern uint32_t nBroadcastMask;
extern uint32_t nOnNetworkMask;
}  // namespace globals

extern void arp_init();
extern void ip_set_ip();

static netif_ext_callback_fn callback_fn;

static void default_callback([[maybe_unused]] const uint16_t reason, [[maybe_unused]] const netif_ext_callback_args_t* args) {
	DEBUG_PRINTF("%u", reason);
}

void netif_init() {
	auto &netif = net::globals::netif_default;

	netif.ip.addr = 0;
	netif.netmask.addr = 0;
	netif.gw.addr = 0;
	netif.broadcast_ip.addr = 0;
	netif.secondary_ip.addr = 2
			+ ((static_cast<uint32_t>(netif.hwaddr[3])) << 8)
			+ ((static_cast<uint32_t>(netif.hwaddr[4])) << 16)
			+ ((static_cast<uint32_t>(netif.hwaddr[5])) << 24);
	netif.flags = 0;
	netif.dhcp = nullptr;
	netif.acd = nullptr;
	netif.autoip = nullptr;

	callback_fn = &default_callback;

	net::arp_init();
}

static void netif_do_update_globals() {
	auto &netif = net::globals::netif_default;
	netif.broadcast_ip.addr = (netif.ip.addr | ~netif.netmask.addr);

	globals::nBroadcastMask = ~(netif.netmask.addr);
	globals::nOnNetworkMask =netif.ip.addr & netif.netmask.addr;

	ip_set_ip();
}

static void netif_do_ip_addr_changed([[maybe_unused]] const ip4_addr_t old_addr,[[maybe_unused]] const ip4_addr_t new_addr) {
//  tcp_netif_ip_addr_changed(old_addr, new_addr);
//  udp_netif_ip_addr_changed(old_addr, new_addr);
	ip_set_ip();
}

static void netif_issue_reports() {
	const auto &netif = net::globals::netif_default;

	if (!(netif.flags & netif::NETIF_FLAG_LINK_UP)) {
		return;
	}

	if (netif.ip.addr != 0) {
		igmp_report_groups();
	}
}

static bool netif_do_set_ipaddr(const ip4_addr_t ipaddr, ip4_addr_t &old_addr) {
	DEBUG_ENTRY
	auto &netif = net::globals::netif_default;

	DEBUG_PRINTF(IPSTR " " IPSTR, IP2STR(ipaddr.addr), IP2STR(netif.ip.addr));

	// Update the address if it's different
	if (ipaddr.addr != netif.ip.addr) {
		old_addr.addr = netif.ip.addr;

		netif_do_ip_addr_changed(old_addr, ipaddr);
		acd_netif_ip_addr_changed(old_addr, ipaddr);

		netif.ip.addr = ipaddr.addr;

		netif_do_update_globals();
		netif_issue_reports();

		DEBUG_EXIT
		return true;	// address changed
	}

	DEBUG_EXIT
	return false;	// address unchanged
}

static bool netif_do_set_netmask(const ip4_addr_t netmask, ip4_addr_t &old_nm) {
	DEBUG_ENTRY
	auto &netif = net::globals::netif_default;

	if (netmask.addr != netif.netmask.addr) {
		old_nm.addr = netif.netmask.addr;
		netif.netmask.addr = netmask.addr;

		netif_do_update_globals();

		DEBUG_EXIT
		return true;	// netmask changed
	}

	DEBUG_EXIT
	return false;	// netmask unchanged
}

static bool netif_do_set_gw(const ip4_addr_t gw, ip4_addr_t &old_gw) {
	DEBUG_ENTRY
	auto &netif = net::globals::netif_default;

	if (gw.addr != netif.gw.addr) {
		old_gw.addr = netif.gw.addr;
		netif.gw.addr = gw.addr;

		DEBUG_EXIT
		return true;	// gateway changed
	}

	DEBUG_EXIT
	return false;	// gateway unchanged
}

void netif_set_ipaddr(const ip4_addr_t ipaddr) {
	ip4_addr_t old_addr;

	if (netif_do_set_ipaddr(ipaddr, old_addr)) {
		netif_ext_callback_args_t args;
		args.ipv4_changed.old_address.addr = old_addr.addr;
		callback_fn(NetifReason::NSC_IPV4_ADDRESS_CHANGED, &args);
	}
}

void netif_set_netmask(const ip4_addr_t netmask) {
	ip4_addr_t old_nm;

	if (netif_do_set_netmask(netmask, old_nm)) {
		netif_ext_callback_args_t args;
		args.ipv4_changed.old_netmask = old_nm;
		callback_fn(NetifReason::NSC_IPV4_NETMASK_CHANGED, &args);
	}
}

void netif_set_gw(const ip4_addr_t gw) {
	ip4_addr_t old_gw;

	if (netif_do_set_gw(gw, old_gw)) {
	    netif_ext_callback_args_t args;
	    args.ipv4_changed.old_gw = old_gw;
	    callback_fn(NetifReason::NSC_IPV4_GATEWAY_CHANGED, &args);
	}
}

void netif_set_addr(const ip4_addr_t ipaddr, const ip4_addr_t netmask, const ip4_addr_t gw) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR " " IPSTR " " IPSTR, IP2STR(ipaddr.addr), IP2STR(netmask.addr), IP2STR(gw.addr));

	auto change_reason = NetifReason::NSC_NONE;
	netif_ext_callback_args_t cb_args;

	ip4_addr_t old_addr;
	ip4_addr_t old_nm;
	ip4_addr_t old_gw;

	const auto remove = (ipaddr.addr == 0);

	if (remove) {
		/* when removing an address, we have to remove it *before* changing netmask/gw
		 to ensure that tcp RST segment can be sent correctly */
		if (netif_do_set_ipaddr(ipaddr, old_addr)) {
			change_reason |= NetifReason::NSC_IPV4_ADDRESS_CHANGED;
			cb_args.ipv4_changed.old_address.addr = old_addr.addr;
		}
	}

	if (netif_do_set_netmask(netmask, old_nm)) {
		change_reason |= NetifReason::NSC_IPV4_NETMASK_CHANGED;
		cb_args.ipv4_changed.old_netmask.addr = old_nm.addr;
	}

	if (netif_do_set_gw(gw, old_gw)) {
		change_reason |= NetifReason::NSC_IPV4_GATEWAY_CHANGED;
		cb_args.ipv4_changed.old_gw = old_gw;
	}

	if (!remove) {
		/* set ipaddr last to ensure netmask/gw have been set when status callback is called */
		if (netif_do_set_ipaddr(ipaddr, old_addr)) {
			change_reason |= NetifReason::NSC_IPV4_ADDRESS_CHANGED;
			cb_args.ipv4_changed.old_address.addr = old_addr.addr;
		}
	}

	if (change_reason != NetifReason::NSC_NONE) {
		change_reason |= NetifReason::NSC_IPV4_SETTINGS_CHANGED;
	}

	if (!remove) {
		/* Issue a callback even if the address hasn't changed, eg. DHCP reboot */
		change_reason |= NetifReason::NSC_IPV4_ADDR_VALID;
	}

	DEBUG_PRINTF("change_reason=%u", change_reason);

	if (change_reason != NetifReason::NSC_NONE) {
		callback_fn(change_reason, &cb_args);
	}

	DEBUG_EXIT
}

void netif_add_ext_callback(netif_ext_callback_fn fn) {
	callback_fn = fn;
}

/*
 * Link
 */

void netif_set_link_up() {
	DEBUG_ENTRY
	const auto &netif = net::globals::netif_default;

	if (!(netif.flags & netif::NETIF_FLAG_LINK_UP)) {
		netif_set_flags(netif::NETIF_FLAG_LINK_UP);

		dhcp_network_changed_link_up();

		autoip_network_changed_link_up();

		netif_issue_reports();

		netif_ext_callback_args_t args;
		args.link_changed.state = 1;
		callback_fn(NetifReason::NSC_LINK_CHANGED, &args);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void netif_set_link_down() {
	DEBUG_ENTRY
	const auto &netif = net::globals::netif_default;

	if (netif.flags & netif::NETIF_FLAG_LINK_UP) {
		netif_clear_flags(netif::NETIF_FLAG_LINK_UP);

		autoip_network_changed_link_down();

		acd_network_changed_link_down();

		netif_ext_callback_args_t args;
		args.link_changed.state = 0;
		callback_fn(NetifReason::NSC_LINK_CHANGED, &args);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}
}  // namespace net
