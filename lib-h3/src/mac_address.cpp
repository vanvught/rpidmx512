/**
 * @file mac_address.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "h3.h"

#ifndef NDEBUG
int uart0_printf(const char* fmt, ...);
#endif

void mac_address_get(uint8_t paddr[]) {
	assert(mac_addr == ENET_MAC_ADDRESS0);

	const uint32_t mac_lo = H3_EMAC->ADDR[0].LOW;
	const uint32_t mac_hi = H3_EMAC->ADDR[0].HIGH;

#ifndef NDEBUG
	uart0_printf("H3_EMAC->ADDR[0].LOW=%08x, H3_EMAC->ADDR[0].HIGH=%08x\n", mac_lo, mac_hi);
#endif

	paddr[0] = (mac_lo >> 0) & 0xff;
	paddr[1] = (mac_lo >> 8) & 0xff;
	paddr[2] = (mac_lo >> 16) & 0xff;
	paddr[3] = static_cast<uint8_t> ((mac_lo >> 24) & 0xff);
	paddr[4] = (mac_hi >> 0) & 0xff;
	paddr[5] = (mac_hi >> 8) & 0xff;

#ifndef NDEBUG
	uart0_printf("%02x:%02x:%02x:%02x:%02x:%02x\n", paddr[0], paddr[1], paddr[2], paddr[3], paddr[4], paddr[5]);
#endif
}
