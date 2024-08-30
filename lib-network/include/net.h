/**
 * @file net.h
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

#ifndef NET_H_
#define NET_H_

#include <cstdint>

#include "netif.h"
#include "emac/phy.h"
#include "net/dhcp.h"
#include "net/protocol/dhcp.h"

#include "debug.h"

namespace network {
void mdns_shutdown();
}  // namespace network

namespace net {
void tcp_shutdown();
void igmp_shutdown();
}

namespace net {
void net_init(net::Link link, ip4_addr_t ipaddr, ip4_addr_t netmask, ip4_addr_t gw, bool &bUseDhcp);
void net_set_primary_ip(const ip4_addr_t ipaddr);
void net_set_secondary_ip();
void net_handle();

inline void net_link_down() {
	network::mdns_shutdown();
#if defined (ENABLE_HTTPD)
	tcp_shutdown();
#endif
	igmp_shutdown();
	dhcp_release_and_stop();
}

int udp_begin(uint16_t);
int udp_end(uint16_t);
uint32_t udp_recv1(int, uint8_t *, uint32_t, uint32_t *, uint16_t *);
uint32_t udp_recv2(int, const uint8_t **, uint32_t *, uint16_t *);
void udp_send(int, const uint8_t *, uint32_t, uint32_t, uint16_t);
void udp_send_timestamp(int, const uint8_t *, uint32_t, uint32_t, uint16_t);

void igmp_join(uint32_t);
void igmp_leave(uint32_t);

int tcp_begin(const uint16_t);
uint16_t tcp_read(const int32_t, const uint8_t **, uint32_t &);
void tcp_write(const int32_t, const uint8_t *, uint32_t, const uint32_t);

/**
 * Must be provided by the application
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
}  // namespace het

#endif /* NET_H_ */
