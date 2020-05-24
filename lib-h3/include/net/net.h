/**
 * @file net.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>

struct ip_addr {
    uint32_t addr;
};

typedef struct ip_addr ip_addr_t;

struct ip_info {
    struct ip_addr ip;
    struct ip_addr netmask;
    struct ip_addr gw;
};

#define IP_BROADCAST	((uint32_t) 0xFFFFFFFF)
#define HOST_NAME_MAX 	64	/* including a terminating null byte. */

#ifdef __cplusplus
extern "C" {
#endif

extern void net_init(const uint8_t *, struct ip_info *, const uint8_t *, bool *, bool *);
extern void net_shutdown(void);
extern void net_handle(void);
//
extern void net_set_hostname(const char *name);
extern void net_set_ip(uint32_t);
extern bool net_set_dhcp(struct ip_info *, bool *);
extern bool net_set_zeroconf(struct ip_info *);
//
extern void net_dhcp_release(void);
//
extern int udp_bind(uint16_t);
extern int udp_unbind(uint16_t);
extern uint16_t udp_recv(uint8_t, uint8_t *, uint16_t, uint32_t *, uint16_t *);
extern int udp_send(uint8_t, const uint8_t *, uint16_t, uint32_t, uint16_t);
//
extern int igmp_join(uint32_t);
extern int igmp_leave(uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* NET_H_ */
