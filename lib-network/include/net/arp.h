/**
 * @file arp_private.h
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

#ifndef ARP_PRIVATE_H_
#define ARP_PRIVATE_H_

#include "netif.h"
#include "net/protocol/ip4.h"
#include "net/protocol/arp.h"
#include "net/protocol/udp.h"

namespace net {
namespace arp {
enum class Flags {
	FLAG_INSERT, FLAG_UPDATE
};
}  // namespace arp

void arp_init();
void arp_handle(struct t_arp *);
void arp_send(struct t_udp *, const uint32_t, const uint32_t);
#if defined CONFIG_ENET_ENABLE_PTP
void arp_send_timestamp(struct t_udp *, const uint32_t, const uint32_t);
#endif
void arp_acd_probe(const ip4_addr_t ipaddr);
void arp_acd_send_announcement(const ip4_addr_t ipaddr);
}  // namespace net

#endif /* ARP_PRIVATE_H_ */
