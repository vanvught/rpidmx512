/**
 * @file rfc3927.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "net/net.h"
#include "net_packets.h"
#include "net_debug.h"

#include "h3.h"

extern uint32_t arp_cache_lookup(uint32_t, uint8_t *);

/*
 * https://tools.ietf.org/html/rfc3927
 * Dynamic Configuration of IPv4 Link-Local Addresses
 */


typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static const _pcast32 s_ip_begin __attribute__ ((aligned (4))) = { .u8 = { 169, 254,   0,   1 } };
static const _pcast32 s_ip_end   __attribute__ ((aligned (4))) = { .u8 = { 169, 254, 254, 255 } };

static uint8_t s_mac_address[6] __attribute__ ((aligned (4)));
static uint8_t s_mac_address_arp_reply[6] __attribute__ ((aligned (4)));

void __attribute__((cold)) rfc3927_init(const uint8_t *mac_address) {
	memcpy(s_mac_address, mac_address, ETH_ADDR_LEN);
}

bool rfc3927(struct ip_info *p_ip_info) {
	const uint32_t mask = (uint32_t) s_mac_address[3] + ((uint32_t) s_mac_address[4] << 8);

	uint32_t ip = s_ip_begin.u32 | (mask << 16);

	DEBUG_PRINTF("ip=%p", ip);

	uint16_t count = 0;

	const uint32_t micros_stamp = H3_TIMER->AVS_CNT1;

	do  {
		DEBUG_PRINTF(IPSTR, IP2STR(ip));

		if (0 == arp_cache_lookup(ip, s_mac_address_arp_reply)) {
			p_ip_info->ip.addr = ip;
			p_ip_info->gw.addr = ip;
			p_ip_info->netmask.addr = 0x0000FFFF;

			return true;
		}

		ip = __builtin_bswap32(__builtin_bswap32(ip) + 1);

		if (ip == s_ip_end.u32) {
			ip = s_ip_begin.u32;
		}

		count++;
	} while ((count < 0xFF) && ((H3_TIMER->AVS_CNT1 - micros_stamp) < (500 * 1000)));

	p_ip_info->ip.addr = 0;
	p_ip_info->gw.addr = 0;
	p_ip_info->netmask.addr = 0;

	return false;
}
