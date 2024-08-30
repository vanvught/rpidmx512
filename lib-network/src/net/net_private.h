/**
 * @file net_private.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PRIVATE_H_
#define NET_PRIVATE_H_

#include <cstdint>

#include "net_platform.h"

#include "net/arp.h"
#include "net/protocol/icmp.h"
#include "net/protocol/igmp.h"
#include "net/protocol/udp.h"
#include "net/protocol/tcp.h"

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

namespace net::arp {
enum class EthSend {
	IS_NORMAL
#if defined CONFIG_ENET_ENABLE_PTP
	, IS_TIMESTAMP
#endif
};
} // namespace net::arp

extern "C" void console_error(const char *);

void emac_eth_send(void *, uint32_t);
#if defined CONFIG_ENET_ENABLE_PTP
void emac_eth_send_timestamp(void *, uint32_t);
#endif
int emac_eth_recv(uint8_t **);
void emac_free_pkt();

namespace net {
void net_handle();

uint16_t net_chksum(const void *, uint32_t);
void net_timers_run();

void ip_init();
void ip_set_ip();
void ip_shutdown();
void ip_handle(struct t_ip4 *);

void udp_init();
void udp_set_ip();
void udp_handle(struct t_udp *);
void udp_shutdown();

void igmp_init();
void igmp_set_ip();
void igmp_handle(struct t_igmp *);
void igmp_shutdown();

void icmp_handle(struct t_icmp *);
void icmp_shutdown();

void tcp_init();
void tcp_run();
void tcp_handle(struct t_tcp *);
void tcp_shutdown();
}  // namespace net

#endif /* NET_PRIVATE_H_ */
