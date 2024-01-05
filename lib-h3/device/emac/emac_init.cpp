/**
 * @file emac_init.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3.h"
#include "h3_sid.h"

#include "debug.h"

void _write_hwaddr(const uint8_t *mac_id) {
	const auto macid_lo = static_cast<uint32_t>(mac_id[0]) + static_cast<uint32_t>(mac_id[1] << 8) + static_cast<uint32_t>(mac_id[2] << 16) + static_cast<uint32_t>(mac_id[3] << 24);
	const auto macid_hi = static_cast<uint32_t>(mac_id[4]) + static_cast<uint32_t>(mac_id[5] << 8);

	H3_EMAC->ADDR[0].HIGH = macid_hi;
	H3_EMAC->ADDR[0].LOW = macid_lo;
}

__attribute__((cold)) void emac_init() {
	DEBUG_PRINTF("PHY{%d} ID = %08x", PHY_ADDR, phy_get_id(PHY_ADDR));

	uint8_t mac_address[6];
	uint8_t rootkey[16];

	h3_sid_get_rootkey(rootkey);

	mac_address[0] = 0x2;
	mac_address[1] = rootkey[3];
	mac_address[2] = rootkey[12];
	mac_address[3] = rootkey[13];
	mac_address[4] = rootkey[14];
	mac_address[5] = rootkey[15];

	_write_hwaddr(mac_address);

	DEBUG_PRINTF("H3_EMAC->ADDR[0].LOW=%08x, H3_EMAC->ADDR[0].HIGH=%08x", H3_EMAC->ADDR[0].LOW, H3_EMAC->ADDR[0].HIGH);
}
