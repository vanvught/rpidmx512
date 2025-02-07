/**
 * @file net.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_H_
#define NET_H_

#include <cstdint>

#include "net/netif.h"

namespace net {
void net_set_primary_ip(const uint32_t nIp);
inline uint32_t net_get_primary_ip() { return net::netif_ipaddr(); }

void net_set_secondary_ip();

void net_set_netmask(const uint32_t nNetmask);
inline uint32_t net_get_netmask() { return net::netif_netmask(); }

void net_set_gateway_ip(const uint32_t nNetmask);
inline uint32_t net_get_gateway_ip() { return net::netif_gw(); }

inline bool net_is_dhcp_capable() { return true; }
inline bool net_is_dhcp_known() { return true; }
void net_enable_dhcp();

inline bool net_is_zeroconf_capable() { return true; }
inline bool net_is_zeroconf_used() { return net::netif_autoip(); };
void net_set_zeroconf();

inline char net_get_addressing_mode() {
	if (net::netif_autoip()) {
		return  'Z';
	} else if (net_is_dhcp_known()) {
		if (net::netif_dhcp()) {
			return 'D';
		} else {
			return 'S';
		}
	}

	return 'U';
}

inline bool net_is_valid_ip(const uint32_t nIp) {
	return (net::netif_ipaddr() & net::netif_netmask()) == (nIp & net::netif_netmask());
}
}  // namespace net

#endif /* NET_H_ */
