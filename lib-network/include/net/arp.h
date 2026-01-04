/**
 * @file arp.h
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

#ifndef NET_ARP_H_
#define NET_ARP_H_

#include "net/ip4_address.h"
#include "net/protocol/arp.h"
#include "net/protocol/udp.h"

namespace net::arp
{
enum class Flags
{
    FLAG_INSERT,
    FLAG_UPDATE
};

void Init();
void Input(const struct t_arp*);
void Send(struct t_udp*, const uint32_t, const uint32_t);
#if defined CONFIG_NET_ENABLE_PTP
void SendTimestamp(struct t_udp*, const uint32_t, const uint32_t);
#endif
void AcdProbe(ip4_addr_t ipaddr);
void AcdSendAnnouncement(ip4_addr_t ipaddr);

} // namespace net::arp

#endif  // NET_ARP_H_
