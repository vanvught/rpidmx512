/**
 * @file net.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

struct ip_addr {
    uint32_t addr;
};

typedef struct ip_addr ip_addr_t;

struct IpInfo {
    struct ip_addr ip;
    struct ip_addr netmask;
    struct ip_addr gw;
    struct ip_addr broadcast_ip;
    struct ip_addr secondary_ip;
};

#define IP_BROADCAST	(0xFFFFFFFF)
#define HOST_NAME_MAX 	64	/* including a terminating null byte. */

void net_init(const uint8_t *const, struct IpInfo *, const char *, bool *, bool *);
void net_shutdown();
void net_handle();

void net_set_ip(struct IpInfo *);
void net_set_netmask(struct IpInfo *);
void net_set_gw(struct IpInfo *);
bool net_set_zeroconf(struct IpInfo *);

bool net_set_dhcp(struct IpInfo *, const char *const, bool *);
void net_dhcp_release();

int udp_begin(uint16_t);
int udp_end(uint16_t);
uint16_t udp_recv1(int, uint8_t *, uint16_t, uint32_t *, uint16_t *);
uint16_t udp_recv2(int, const uint8_t **, uint32_t *, uint16_t *);
int udp_send(int, const uint8_t *, uint16_t, uint32_t, uint16_t);

void igmp_join(uint32_t);
void igmp_leave(uint32_t);

int tcp_begin(const uint16_t);
uint16_t tcp_read(const int32_t, const uint8_t **, uint32_t &);
void tcp_write(const int32_t, const uint8_t *, uint16_t, const uint32_t);

#endif /* NET_H_ */
