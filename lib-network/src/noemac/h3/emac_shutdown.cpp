/**
 * @file emac_shutdown.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3.h"
#include "../../emac/h3/emac.h"

#include "firmware/debug/debug_debug.h"

#define H3_EPHY_SHUTDOWN				(1U << 16) 	// 1: shutdown, 0: power up

extern "C" __attribute__((cold)) void emac_shutdown(void) {
	uint32_t value;

	value = H3_EMAC->RX_CTL0;
	value &= ~RX_CTL0_RX_EN;
	H3_EMAC->RX_CTL0 = value;

	value = H3_EMAC->TX_CTL0;
	value &= ~TX_CTL0_TX_EN;
	H3_EMAC->TX_CTL0 = value;

	value = H3_EMAC->TX_CTL1;
	value &= (uint32_t)~TX_CTL1_TX_DMA_EN;
	H3_EMAC->TX_CTL1 = value;

	value = H3_EMAC->RX_CTL1;
	value &= (uint32_t)~RX_CTL1_RX_DMA_EN;
	H3_EMAC->RX_CTL1 = value;

	H3_CCU->BUS_CLK_GATING4 &= (uint32_t)~BUS_CLK_GATING4_EPHY_GATING;

	H3_SYSTEM->EMAC_CLK |= H3_EPHY_SHUTDOWN;

#if 0
	//-> The below gives continues leds on
	H3_CCU->BUS_SOFT_RESET2 &= ~BUS_SOFT_RESET2_EPHY_RST;

	net::phy_shutdown(PHY_ADDR);
	//<-
#endif
}
