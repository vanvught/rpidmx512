/**
 * @file netif.h
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

#ifndef NETIF_H_
#define NETIF_H_

#include <cstdint>

#include "netif.h"
#include "ip4_address.h"

#include "debug.h"

#ifndef NETIF_MAX_HWADDR_LEN
# define NETIF_MAX_HWADDR_LEN 6U
#endif

namespace net {
struct ip_addr {
	uint32_t addr;
};

typedef struct ip_addr ip4_addr_t;

struct netif {
	static constexpr uint8_t NETIF_FLAG_LINK_UP = (1U << 0);
	static constexpr uint8_t NETIF_FLAG_DHCP_OK = (1U << 1);
	static constexpr uint8_t NETIF_FLAG_AUTOIP_OK = (1U << 2);
	static constexpr uint8_t NETIF_FLAG_STATICIP_OK = (1U << 3);

	struct ip_addr ip;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct ip_addr broadcast_ip;
	struct ip_addr secondary_ip;

	uint8_t hwaddr[NETIF_MAX_HWADDR_LEN];
	uint8_t flags;

	const char *hostname;

	void *dhcp;
	void *acd;
	void *autoip;
};

struct NetifReason {
	static constexpr uint16_t NSC_NONE					= 0x0000;
	static constexpr uint16_t NSC_LINK_CHANGED			= 0x0004;
	static constexpr uint16_t NSC_IPV4_ADDRESS_CHANGED	= 0x0010;
	static constexpr uint16_t NSC_IPV4_GATEWAY_CHANGED	= 0x0020;
	static constexpr uint16_t NSC_IPV4_NETMASK_CHANGED	= 0x0040;
	static constexpr uint16_t NSC_IPV4_SETTINGS_CHANGED	= 0x0080;
	static constexpr uint16_t NSC_IPV4_ADDR_VALID 		= 0x0400;
};

struct ipv4_changed {
	ip4_addr_t old_address;
	ip4_addr_t old_netmask;
	ip4_addr_t old_gw;
};

struct link_changed {
	/** 1: up; 0: down */
	uint8_t state;
} ;

union netif_ext_callback_args_t {
	struct link_changed link_changed;
	struct ipv4_changed ipv4_changed;
};

namespace globals {
extern struct netif netif_default;
}  // namespace globals

typedef void (* netif_ext_callback_fn)(const uint16_t reason, const netif_ext_callback_args_t *args);

inline void netif_set_flags(uint8_t flags) {
	globals::netif_default.flags |= flags;
}

inline void netif_clear_flags(uint8_t flags) {
	globals::netif_default.flags &= static_cast<uint8_t>(~flags);
}

inline bool netif_dhcp() {
	return (globals::netif_default.flags & netif::NETIF_FLAG_DHCP_OK) == netif::NETIF_FLAG_DHCP_OK;
}

inline bool netif_autoip() {
	return (globals::netif_default.flags & netif::NETIF_FLAG_AUTOIP_OK) == netif::NETIF_FLAG_AUTOIP_OK;
}

inline uint32_t netif_ipaddr() {
	return globals::netif_default.ip.addr;
}

inline uint32_t netif_netmask() {
	return globals::netif_default.netmask.addr;
}

inline uint32_t netif_gw() {
	return globals::netif_default.gw.addr;
}

inline void netif_set_hostname(const char *hostname) {
	globals::netif_default.hostname = hostname;
}

inline const char *netif_hostname() {
	return globals::netif_default.hostname;
}

inline uint32_t netif_secondary_ipaddr() {
	return globals::netif_default.secondary_ip.addr;
}

inline uint32_t netif_broadcast_ipaddr() {
	return globals::netif_default.broadcast_ip.addr;
}

inline const uint8_t *netif_hwaddr() {
	return globals::netif_default.hwaddr;
}
void netif_init();
void netif_set_ipaddr(const ip4_addr_t ipaddr);
void netif_set_netmask(const ip4_addr_t netmask);
void netif_set_gw(const ip4_addr_t gw);
void netif_set_addr(const ip4_addr_t ipaddr, const ip4_addr_t netmask, const ip4_addr_t gw);

void netif_add_ext_callback(netif_ext_callback_fn fn);

/**
 * Link
 */

void netif_set_link_up();
void netif_set_link_down();

inline bool netif_is_link_up() {
	return (globals::netif_default.flags & netif::NETIF_FLAG_LINK_UP) == netif::NETIF_FLAG_LINK_UP;
}
}  // namespace net

#endif /* NETIF_H_ */
