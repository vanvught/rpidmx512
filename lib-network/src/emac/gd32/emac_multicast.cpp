/**
 * @file emac_multicast.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_EMAC_IGMP)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstddef>

#include "gd32.h"
#include "gd32_enet.h"
#include "net/ip4_address.h"

#include "debug.h"

uint32_t ethcrc(const uint8_t *data, const size_t length);

void emac_multicast_enable_hash_filter() {
	DEBUG_ENTRY

	gd32_enet_reset_hash();
	gd32_enet_filter_feature_disable<ENET_MULTICAST_FILTER_PASS>();
	gd32_enet_filter_feature_enable<ENET_MULTICAST_FILTER_HASH_MODE>();

	DEBUG_EXIT
}
void emac_multicast_disable_hash_filter() {
	DEBUG_ENTRY

	gd32_enet_filter_feature_disable<ENET_MULTICAST_FILTER_HASH_MODE>();
	gd32_enet_filter_feature_enable<ENET_MULTICAST_FILTER_PASS>();

	DEBUG_EXIT
}

void emac_multicast_set_hash(const uint8_t *mac_addr) {
	DEBUG_ENTRY

	const auto crc = ethcrc(mac_addr, 6);
	const auto hash = (crc >> 26) & 0x3F;

	gd32_enet_filter_set_hash(hash);

	DEBUG_PRINTF("MAC: " MACSTR " -> CRC32: 0x%08X -> Hash Index: %d", MAC2STR(mac_addr), crc, hash);
	DEBUG_EXIT
}

void emac_multicast_reset_hash() {
	DEBUG_ENTRY

	gd32_enet_reset_hash();

	DEBUG_EXIT
}
