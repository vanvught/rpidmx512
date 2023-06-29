/**
 * @file arp_cache.cpp
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "net_private.h"
#include "net_packets.h"
#include "net_platform.h"
#include "net_debug.h"

#include "emac/net_link_check.h"

#include "../../config/net_config.h"

#if !defined ARP_MAX_RECORDS
static constexpr auto MAX_RECORDS = 32;
#else
static constexpr auto MAX_RECORDS = ARP_MAX_RECORDS;
#endif

struct ArpRecord {
	uint32_t nIp;
	uint8_t mac_address[ETH_ADDR_LEN];
};

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static ArpRecord s_ArpRecords[MAX_RECORDS] SECTION_NETWORK ALIGNED;
static uint16_t s_Entries SECTION_NETWORK ALIGNED;

#ifndef NDEBUG
# define TICKER_COUNT 100	///< 10 seconds
  static volatile uint32_t s_ticker ;
#endif

void __attribute__((cold)) arp_cache_init() {
	s_Entries = 0;

	for (auto& record : s_ArpRecords) {
		memset(&record, 0, sizeof(struct ArpRecord));
	}

#ifndef NDEBUG
	s_ticker = TICKER_COUNT;
#endif
}

void arp_cache_update(const uint8_t *pMacAddress, uint32_t nIp) {
	DEBUG_ENTRY
	DEBUG_PRINTF(MACSTR " " IPSTR, MAC2STR(pMacAddress), IP2STR(nIp));

	if (s_Entries == MAX_RECORDS) {
		console_error("ARP cache is full\n");
		return;
	}

	for (auto i = 0; i < s_Entries; i++) {
		if (s_ArpRecords[i].nIp == nIp) {
			DEBUG_EXIT
			return;
		}
	}

	memcpy(s_ArpRecords[s_Entries].mac_address, pMacAddress, ETH_ADDR_LEN);
	s_ArpRecords[s_Entries].nIp = nIp;

	s_Entries++;

	DEBUG_EXIT
}

uint32_t arp_cache_lookup(uint32_t nIp, uint8_t *pMacAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(nIp), MAC2STR(pMacAddress));

	uint32_t i;

	for (i = 0; i < MAX_RECORDS; i++) {
		if (s_ArpRecords[i].nIp == nIp) {
			memcpy(pMacAddress, s_ArpRecords[i].mac_address, ETH_ADDR_LEN);
			DEBUG_EXIT
			return nIp;
		}

		if (s_ArpRecords[i].nIp == 0) {
			break;
		}
	}

	if (net::link_status_read() == net::Link::STATE_DOWN) {
		DEBUG_EXIT
		return 0;
	}

	const auto nEntries = s_Entries;
	int32_t nTimeout;
	auto nRetries = 3;

	while (nRetries--) {
		arp_send_request(nIp);

		nTimeout = 0x1FFFF;
#ifndef NDEBUG
		nTimeout+= 0x40000;
#endif

		while ((nTimeout-- > 0) && (nEntries == s_Entries)) {
			net_handle();
		}

		if (nEntries != s_Entries) {
			memcpy(pMacAddress, s_ArpRecords[nEntries].mac_address, ETH_ADDR_LEN);
			DEBUG_PRINTF("timeout=%x", nTimeout);
			DEBUG_EXIT
			return nIp;
		}

		DEBUG_PRINTF("timeout=%d, current_entry=%d, s_entry_current=%d", nTimeout, nEntries, s_Entries);
	}

	DEBUG_EXIT
	return 0;
}

void arp_cache_dump() {
#ifndef NDEBUG
	printf("ARP Cache size=%d\n", s_Entries);

	for (auto i = 0; i < s_Entries; i++) {
		printf("%02d " IPSTR " " MACSTR "\n", i, IP2STR(s_ArpRecords[i].nIp),MAC2STR(s_ArpRecords[i].mac_address));
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
