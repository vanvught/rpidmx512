/**
 * link_handle_change.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hardware.h"
#include "network.h"
#include "netif.h"

#include "debug.h"

#if !defined(PHY_ADDRESS)
# define PHY_ADDRESS	1
#endif

extern void emac_adjust_link(const net::PhyStatus);

namespace net {
void link_handle_change(const net::Link state) {
	DEBUG_PRINTF("net::Link %s", state == net::Link::STATE_UP ? "UP" : "DOWN");

	if (Link::STATE_UP == state) {
		const bool isWatchdog = Hardware::Get()->IsWatchdog();
		if (isWatchdog) {
			Hardware::Get()->WatchdogStop();
		}

		PhyStatus phyStatus;
		phy_start(PHY_ADDRESS, phyStatus);

		emac_adjust_link(phyStatus);

		if (isWatchdog) {
			Hardware::Get()->WatchdogInit();
		}

		netif_set_link_up();
		return;
	}

	netif_set_link_down();
}
}  // namespace net
