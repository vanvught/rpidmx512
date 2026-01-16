/**
 * @file phy.cpp
 *
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "firmware/debug/debug_debug.h"

#if !defined(BIT)
#define BIT(x) static_cast<uint16_t>(1U << (x))
#endif

#define PHY_REG_RMSR 0x10
#define PHY_REG_PAGE_SELECT 0x1f
#define PHY_REG_IER 0x13
#define IER_CUSTOM_LED (1U << 3)

#if !defined(PHY_ADDRESS)
#define PHY_ADDRESS 1
#endif

namespace net::phy
{
void WritePaged(uint16_t phy_page, uint16_t phy_reg, uint16_t phy_value, uint16_t mask = 0x0)
{
    phy::Write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, phy_page);

    uint16_t tmp_value;
    phy::Read(PHY_ADDRESS, phy_reg, tmp_value);
    DEBUG_PRINTF("tmp_value=0x%.4x, mask=0x%.4x", tmp_value, mask);
    tmp_value &= static_cast<uint16_t>(~mask);
    tmp_value |= phy_value;
    DEBUG_PRINTF("tmp_value=0x%.4x, phy_value=0x%.4x", tmp_value, phy_value);

    phy::Write(PHY_ADDRESS, phy_reg, tmp_value);
    phy::Write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, 0);
}

static void ReadPaged(uint16_t phy_page, uint16_t phy_reg, uint16_t& phy_value, uint16_t mask = 0x0)
{
    phy::Write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, phy_page);
    phy::Read(PHY_ADDRESS, phy_reg, phy_value);
    phy_value &= mask;
    phy::Write(PHY_ADDRESS, PHY_REG_PAGE_SELECT, 0);
}

void CustomizedLed()
{
    DEBUG_ENTRY();

#if defined(RTL8201F_LED1_LINK_ALL) || defined(RTL8201F_LED1_LINK_ALL_ACT)
    WritePaged(0x07, PHY_REG_IER, IER_CUSTOM_LED, IER_CUSTOM_LED);
#if defined(RTL8201F_LED1_LINK_ALL)
    WritePaged(0x07, 0x11, (1U << 3) | (1U << 4) | (1U << 5));
#else
    WritePaged(0x07, 0x11, (1U << 3) | (1U << 4) | (1U << 5) | (1U << 7));
#endif
#endif
    DEBUG_EXIT();
}

#define RMSR_RX_TIMING_SHIFT 4
#define RMSR_RX_TIMING_MASK 0xF0

#define RMSR_TX_TIMING_SHIFT 8
#define RMSR_TX_TIMING_MASK 0xF00

void CustomizedTiming()
{
    DEBUG_ENTRY();
#if defined(GD32F4XX)
#define RMSR_RX_TIMING_VAL 0x4
#if defined(GD32F407)
#define RMSR_TX_TIMING_VAL 0x2 // The GD32F407 is now running at 200MHz
#elif defined(GD32F470)
#define RMSR_TX_TIMING_VAL 0x1
#else
#define RMSR_TX_TIMING_VAL 0xF
#endif

    constexpr uint16_t phy_value = (RMSR_RX_TIMING_VAL << RMSR_RX_TIMING_SHIFT) | (RMSR_TX_TIMING_VAL << RMSR_TX_TIMING_SHIFT);
    WritePaged(0x7, PHY_REG_RMSR, phy_value, RMSR_RX_TIMING_MASK | RMSR_TX_TIMING_MASK);
#endif
    DEBUG_EXIT();
}

void CustomizedStatus(phy::Status& phy_status)
{
    phy_status.link = net::link::StatusRead();

    uint16_t value;
    phy::Read(PHY_ADDRESS, mmi::REG_BMCR, value);

    phy_status.duplex = ((value & BIT(8)) == BIT(8)) ? phy::Duplex::kDuplexFull : phy::Duplex::kDuplexHalf;
    phy_status.speed = ((value & BIT(13)) == BIT(13)) ? phy::Speed::kSpeed100 : phy::Speed::kSpeed10;
    phy_status.autonegotiation = ((value & mmi::BMCR_AUTONEGOTIATION) == mmi::BMCR_AUTONEGOTIATION);
}

namespace rtl8201f
{
void GetTimings(uint32_t& rx_timing, uint32_t& tx_timing)
{
    uint16_t value;
    ReadPaged(0x7, PHY_REG_RMSR, value, RMSR_RX_TIMING_MASK | RMSR_TX_TIMING_MASK);

    rx_timing = (value >> RMSR_RX_TIMING_SHIFT) & 0xF;
    tx_timing = (value >> RMSR_TX_TIMING_SHIFT) & 0xF;
}

void SetRxtiming(uint32_t rx_timing)
{
    const auto kValue = static_cast<uint16_t>((rx_timing & 0xF) << RMSR_RX_TIMING_SHIFT);
    WritePaged(0x7, PHY_REG_RMSR, kValue, RMSR_RX_TIMING_MASK);
}

void SetTxtiming(uint32_t tx_timing)
{
    const auto kValue = static_cast<uint16_t>((tx_timing & 0xF) << RMSR_TX_TIMING_SHIFT);
    WritePaged(0x7, PHY_REG_RMSR, kValue, RMSR_TX_TIMING_MASK);
}
} // namespace rtl8201f
} // namespace net::phy
