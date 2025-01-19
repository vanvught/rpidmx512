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

#include "emac/phy.h"
#include "net/netif.h"
#include "net/dhcp.h"
#include "net/protocol/dhcp.h"

namespace net {
void net_init(Link link, ip4_addr_t ipaddr, ip4_addr_t netmask, ip4_addr_t gw, bool &bUseDhcp);
void net_set_primary_ip(const ip4_addr_t ipaddr);
void net_set_secondary_ip();
void net_handle();
void net_link_down();

/**
 * Must be provided by the user application
 */
void display_emac_config();
void display_emac_start();
void display_emac_status(const bool);
void display_emac_shutdown();
void display_ip();
void display_netmask();
void display_gateway();
void display_hostname();
void display_dhcp_status(net::dhcp::State);
}  // namespace net

#endif /* NET_H_ */
