/**
 * rtl8201f.cpp
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

#include "gd32.h"
#include "./../enet_config.h"

#include "debug.h"

namespace net {
void phy_write_paged(uint16_t phy_page, uint16_t phy_reg, uint16_t phy_value, uint16_t mask = 0x0) {
	enet_phy_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_PAGE_SELECT, &phy_page);

	uint16_t tmp_value;
	enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, phy_reg, &tmp_value);
	DEBUG_PRINTF("tmp_value=0x%.4x, mask=0x%.4x", tmp_value, mask);
	tmp_value &= ~mask;
	tmp_value |= phy_value;
	DEBUG_PRINTF("tmp_value=0x%.4x, phy_value=0x%.4x", tmp_value, phy_value);

	enet_phy_write_read(ENET_PHY_WRITE, PHY_ADDRESS, phy_reg, &tmp_value);

	phy_page = 0;
	enet_phy_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_PAGE_SELECT, &phy_page);
}

void phy_customized_led() {
	DEBUG_ENTRY

#if defined (RTL8201F_LED1_LINK_ALL) || defined (RTL8201F_LED1_LINK_ALL_ACT)
	phy_write_paged(0x07, PHY_REG_IER, PHY_REG_IER_CUSTOM_LED, PHY_REG_IER_CUSTOM_LED);
# if defined (RTL8201F_LED1_LINK_ALL)
	phy_write_paged(0x07, 0x11, BIT(3) | BIT(4) | BIT(5));
# else
	phy_write_paged(0x07, 0x11, BIT(3) | BIT(4) | BIT(5) | BIT(7));
# endif
#endif
	DEBUG_EXIT
}

void phy_customized_timing() {
	DEBUG_ENTRY
#if defined (GD32F4XX)
# define RMSR_RX_TIMING_SHIFT	4
# define RMSR_RX_TIMING_MASK	0xF0
# define RMSR_RX_TIMING_VAL		0x4
# define RMSR_TX_TIMING_SHIFT	8
# define RMSR_TX_TIMING_MASK	0xF00
# define RMSR_TX_TIMING_VAL		0xF

	constexpr uint16_t phy_value = (RMSR_RX_TIMING_VAL << RMSR_RX_TIMING_SHIFT)
				       | (RMSR_TX_TIMING_VAL << RMSR_TX_TIMING_SHIFT);
	phy_write_paged(0x7, PHY_REG_RMSR, phy_value, RMSR_RX_TIMING_MASK | RMSR_TX_TIMING_MASK);
#endif
	DEBUG_EXIT
}
}  // namespace net
