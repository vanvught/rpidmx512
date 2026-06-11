/**
 * @file mac_address.cpp
 *
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
int uart0::Printf(const char* fmt, ...);
#endif

void MacAddress(uint8_t paddr[]) {
	assert(mac_addr == ENET_MAC_ADDRESS0);

	const auto kMacLo = H3_EMAC->ADDR[0].LOW;
	const auto kMacHi = H3_EMAC->ADDR[0].HIGH;

#ifndef NDEBUG
	uart0::Printf("H3_EMAC->ADDR[0].LOW=%08x, H3_EMAC->ADDR[0].HIGH=%08x\n", mac_lo, mac_hi);
#endif

	paddr[0] = (kMacLo >> 0) & 0xff;
	paddr[1] = (kMacLo >> 8) & 0xff;
	paddr[2] = (kMacLo >> 16) & 0xff;
	paddr[3] = static_cast<uint8_t> ((kMacLo >> 24) & 0xff);
	paddr[4] = (kMacHi >> 0) & 0xff;
	paddr[5] = (kMacHi >> 8) & 0xff;

#ifndef NDEBUG
	uart0::Printf("%02x:%02x:%02x:%02x:%02x:%02x\n", paddr[0], paddr[1], paddr[2], paddr[3], paddr[4], paddr[5]);
#endif
}
