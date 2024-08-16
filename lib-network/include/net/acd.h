/**
 * @file acd.h
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
/* This code is inspired by: lwIP
 * https://savannah.nongnu.org/projects/lwip/
 */

#ifndef NET_ACD_H_
#define NET_ACD_H_

#include <cstdint>

#include "netif.h"
#include "arp.h"
#include "net/protocol/acd.h"
#include "ip4_address.h"

/**
 * https://datatracker.ietf.org/doc/html/rfc5227.html
 * IPv4 Address Conflict Detection
 */

namespace net {
typedef void (*acd_conflict_callback_t)(acd::Callback callback);

namespace acd {
struct Acd {
	ip4_addr_t ipaddr;
	State state;
	uint8_t sent_num;
	uint8_t lastconflict;
	uint8_t num_conflicts;
	acd_conflict_callback_t acd_conflict_callback;
	uint16_t ttw;
};
}  // namespace acd

void acd_add(struct acd::Acd *, acd_conflict_callback_t);
void acd_remove(struct acd::Acd *);

void acd_start(struct acd::Acd *, const ip4_addr_t ipaddr);
void acd_stop(struct acd::Acd *);

void acd_arp_reply(struct t_arp *);

void acd_network_changed_link_down();
void acd_netif_ip_addr_changed(const ip4_addr_t nOldIpAddress, const ip4_addr_t nNewIpAddress);
}  // namespace net

#endif /* NET_ACD_H_ */
