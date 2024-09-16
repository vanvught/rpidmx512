/**
 * phy.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "emac/phy.h"
#include "emac/net_link_check.h"
#include "emac/mmi.h"

#include "debug.h"

#if !defined (BIT)
# define BIT(x) static_cast<uint16_t>(1U<<(x))
#endif

#if !defined(PHY_ADDRESS)
# define PHY_ADDRESS 1
#endif

#define PHY_REG_RMSR				0x10
#define PHY_REG_PAGE_SELECT			0x1f
#define PHY_REG_IER					0x13
	#define IER_CUSTOM_LED			(1U << 3)

namespace net {
void phy_write_paged(uint16_t phy_page, uint16_t phy_reg, uint16_t phy_value, uint16_t mask = 0x0) {
	phy_write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, phy_page);

	uint16_t tmp_value;
	phy_read(PHY_ADDRESS, phy_reg, tmp_value);
	DEBUG_PRINTF("tmp_value=0x%.4x, mask=0x%.4x", tmp_value, mask);
	tmp_value &= static_cast<uint16_t>(~mask);
	tmp_value |= phy_value;
	DEBUG_PRINTF("tmp_value=0x%.4x, phy_value=0x%.4x", tmp_value, phy_value);

	phy_write(PHY_ADDRESS, phy_reg, tmp_value);
	phy_write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, 0);
}

void phy_read_paged(const uint16_t phy_page, const uint16_t phy_reg, uint16_t& phy_value, const uint16_t mask = 0x0) {
	phy_write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, phy_page);

	phy_read(PHY_ADDRESS, phy_reg, phy_value);
	phy_value &= mask;

	phy_write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, 0);
}

void phy_customized_led() {
	DEBUG_ENTRY

#if defined (RTL8201F_LED1_LINK_ALL) || defined (RTL8201F_LED1_LINK_ALL_ACT)
	phy_write_paged(0x07, PHY_REG_IER, IER_CUSTOM_LED, IER_CUSTOM_LED);
# if defined (RTL8201F_LED1_LINK_ALL)
	phy_write_paged(0x07, 0x11, (1U << 3) | (1U << 4) | (1U << 5));
# else
	phy_write_paged(0x07, 0x11, (1U << 3) | (1U << 4) | (1U << 5) | (1U << 7));
# endif
#endif
	DEBUG_EXIT
}

#define RMSR_RX_TIMING_SHIFT	4
#define RMSR_RX_TIMING_MASK		0xF0

#define RMSR_TX_TIMING_SHIFT	8
#define RMSR_TX_TIMING_MASK		0xF00

void phy_customized_timing() {
	DEBUG_ENTRY
#if defined (GD32F4XX)
# define RMSR_RX_TIMING_VAL		0x4
# if defined (GD32F407)
#  define RMSR_TX_TIMING_VAL	0x2	// The GD32F407 is now running at 200MHz
# elif defined (GD32F470)
#  define RMSR_TX_TIMING_VAL	0x1
# else
#  define RMSR_TX_TIMING_VAL	0xF
# endif

	constexpr uint16_t phy_value = (RMSR_RX_TIMING_VAL << RMSR_RX_TIMING_SHIFT)
								 | (RMSR_TX_TIMING_VAL << RMSR_TX_TIMING_SHIFT);
	phy_write_paged(0x7, PHY_REG_RMSR, phy_value, RMSR_RX_TIMING_MASK | RMSR_TX_TIMING_MASK);
#endif
	DEBUG_EXIT
}

void phy_customized_status(PhyStatus& phyStatus) {
	phyStatus.link = link_status_read();

	uint16_t nValue;
	phy_read(PHY_ADDRESS, mmi::REG_BMCR, nValue);

	phyStatus.duplex = ((nValue & BIT(8)) == BIT(8)) ? Duplex::DUPLEX_FULL : Duplex::DUPLEX_HALF;
	phyStatus.speed = ((nValue & BIT(13)) == BIT(13)) ? Speed::SPEED100 : Speed::SPEED10;
	phyStatus.bAutonegotiation = ((nValue & mmi::BMCR_AUTONEGOTIATION) == mmi::BMCR_AUTONEGOTIATION);

}
namespace phy {
void rtl8201f_get_timings(uint32_t& nRxTiming, uint32_t& nTxTiming) {
	uint16_t  nValue;
	phy_read_paged(0x7, PHY_REG_RMSR, nValue, RMSR_RX_TIMING_MASK | RMSR_TX_TIMING_MASK);

	nRxTiming = (nValue >> RMSR_RX_TIMING_SHIFT) & 0xF;
	nTxTiming = (nValue >> RMSR_TX_TIMING_SHIFT) & 0xF;
}

void rtl8201f_set_rxtiming(const uint32_t nRxTiming) {
	const auto nValue = static_cast<uint16_t>((nRxTiming & 0xF) << RMSR_RX_TIMING_SHIFT);
	phy_write_paged(0x7, PHY_REG_RMSR, nValue, RMSR_RX_TIMING_MASK);
}

void rtl8201f_set_txtiming(const uint32_t nTxTiming) {
	const auto nValue = static_cast<uint16_t>((nTxTiming & 0xF) << RMSR_TX_TIMING_SHIFT);
	phy_write_paged(0x7, PHY_REG_RMSR, nValue, RMSR_TX_TIMING_MASK);
}

}  // namespace phy
}  // namespace net
