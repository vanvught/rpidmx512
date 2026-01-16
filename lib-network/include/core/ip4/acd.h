/**
 * @file acd.h
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_IP4_ACD_H_
#define CORE_IP4_ACD_H_

#include <cstdint>

#include "core/protocol/arp.h"
#include "core/protocol/acd.h"
#include "ip4/ip4_address.h"

// https://datatracker.ietf.org/doc/html/rfc5227.html
// IPv4 Address Conflict Detection

namespace network::acd
{
typedef void (*conflict_callback_t)(acd::Callback callback);

struct Acd
{
    ip4_addr_t ipaddr;
    State state;
    uint8_t sent_num;
    uint8_t lastconflict;
    uint8_t num_conflicts;
    conflict_callback_t conflict_callback;
    uint16_t ttw;
};

void Add(struct acd::Acd*, conflict_callback_t);
void Remove(struct acd::Acd*);

void Start(struct acd::Acd*, ip4_addr_t ipaddr);
void Stop(struct acd::Acd*);

void ArpReply(const struct network::arp::Header*);

void NetworkChangedLinkDown();
void NetifIpAddrChanged(ip4_addr_t old_ip_address, ip4_addr_t new_ip_address);
} // namespace network::acd

#endif // CORE_IP4_ACD_H_
