/**
 * @file arp_cache.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "net_packets.h"
#include "net_debug.h"

#include "util.h"

extern void arp_send_request(uint32_t ip);
extern void net_handle(void);

#define MAX_RECORDS	32

struct t_arp_record {
	uint32_t ip;
	uint8_t mac_address[ETH_ADDR_LEN];
} ALIGNED;

typedef union pcast32 {
		uint32_t u32;
		uint8_t u8[4];
} _pcast32;

static struct t_arp_record s_arp_records[MAX_RECORDS] ALIGNED;
static uint16_t s_entry_current;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] = {0x01, 0x00, 0x5E}; // Fixed part

#ifndef NDEBUG
 #define TICKER_COUNT 100	///< 10 seconds
 static volatile uint32_t s_ticker ;
#endif

void arp_cache_init(void) {
	uint16_t i;

	s_entry_current = 0;

	for (i = 0; i < MAX_RECORDS; i++) {
		s_arp_records[i].ip = 0;
		memset(s_arp_records[i].mac_address, 0, ETH_ADDR_LEN);
	}

#ifndef NDEBUG
	s_ticker = TICKER_COUNT;
#endif
}

void arp_cache_update(uint8_t *mac_address, uint32_t ip) {
	DEBUG2_ENTRY
	uint16_t i;

	if (s_entry_current == MAX_RECORDS) {
		assert(0);
		return;
	}

	for (i = 0; i < s_entry_current; i++) {
		if (s_arp_records[i].ip == ip) {
			return;
		}
	}

	memcpy(s_arp_records[s_entry_current].mac_address, mac_address, ETH_ADDR_LEN);
	s_arp_records[s_entry_current].ip = ip;

	s_entry_current++;

	DEBUG2_EXIT
}

uint32_t arp_cache_lookup(uint32_t ip, uint8_t *mac_address) {
	DEBUG2_ENTRY

	if ((ip & 0xE0) == 0xE0) { // Multicast, we know the MAC Address
		_pcast32 multicast_ip;

		multicast_ip.u32 = ip;

		s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
		s_multicast_mac[4] = multicast_ip.u8[2];
		s_multicast_mac[5] = multicast_ip.u8[3];

		memcpy(mac_address, s_multicast_mac, ETH_ADDR_LEN);
		return ip;
	}

	DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(ip), MAC2STR(mac_address));

	uint16_t i;

	for (i = 0; i < MAX_RECORDS; i++) {
		if (s_arp_records[i].ip == ip) {
			memcpy(mac_address, s_arp_records[i].mac_address, ETH_ADDR_LEN);
			return ip;
		}

		if (s_arp_records[i].ip == 0) {
			break;
		}
	}

	uint16_t current_entry = s_entry_current;
	int16_t timeout;
	int8_t retries = 3;

	while (retries--) {
		arp_send_request(ip);

		timeout = 0xFFF;

		while ((timeout-- > 0) && (current_entry == s_entry_current)) {
			net_handle();
		}

		if (current_entry != s_entry_current) {
			memcpy(mac_address, s_arp_records[current_entry].mac_address, ETH_ADDR_LEN);
			DEBUG_PRINTF("timeout=%x", timeout);
			return ip;
		}

		DEBUG_PRINTF("i=%d, timeout=%d, current_entry=%d, s_entry_current=%d", i, timeout, current_entry, s_entry_current);
	}

	DEBUG2_EXIT
	return 0;
}

void arp_cache_dump(void) {
#ifndef NDEBUG
	uint16_t i;

	printf("ARP Cache size=%d\n", s_entry_current);

	for (i = 0; i < s_entry_current; i++) {
		printf("%02d " IPSTR " " MACSTR "\n", i, IP2STR(s_arp_records[i].ip),MAC2STR(s_arp_records[i].mac_address));
	}
#endif
}

#ifndef NDEBUG
void arp_cache_timer(void) {
	s_ticker--;

	if (s_ticker == 0) {
		s_ticker = TICKER_COUNT;
		arp_cache_dump();
	}
}
#endif

