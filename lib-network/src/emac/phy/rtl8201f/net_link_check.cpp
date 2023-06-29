/**
 * net_link_check.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "emac/net_link_check.h"
#include "emac/phy.h"
#include "emac/mmi.h"

#if !defined (PHY_ADDRESS)
# define PHY_ADDRESS 1
#endif

#define PHY_REG_IER				0x13
	#define IER_INT_ENABLE		(1U << 13)

#define PHY_REG_ISR				0x1e
	#define ISR_LINK			(1U << 11)

namespace net {
#if defined (ENET_LINK_CHECK_USE_INT) || defined (ENET_LINK_CHECK_USE_PIN_POLL)
void phy_write_paged(uint16_t phy_page, uint16_t phy_reg, uint16_t phy_value, uint16_t mask = 0x0);

void link_pin_enable() {
	phy_write_paged(0x07, PHY_REG_IER, IER_INT_ENABLE, IER_INT_ENABLE);
	// Clear interrupt
	uint16_t phy_value;
	phy_read(PHY_ADDRESS, PHY_REG_ISR, phy_value);
}

void link_pin_recovery() {
    uint16_t phy_value;
    phy_read(PHY_ADDRESS, PHY_REG_ISR, phy_value);
    phy_read(PHY_ADDRESS, mmi::REG_BMSR, phy_value);
}
#endif
}  // namespace net
