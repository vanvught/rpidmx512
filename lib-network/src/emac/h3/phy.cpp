/**
 * @file phy.cpp
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "emac/mmi.h"
 #include "firmware/debug/debug_debug.h"

namespace net::phy
{

#define CONFIG_SYS_HZ 1000
#define CONFIG_MDIO_TIMEOUT (3 * CONFIG_SYS_HZ)

#define MDIO_CMD_MII_BUSY (1U << 0)
#define MDIO_CMD_MII_WRITE (1U << 1)

#define MDIO_CMD_MII_PHY_REG_ADDR_MASK 0x000001f0
#define MDIO_CMD_MII_PHY_REG_ADDR_SHIFT 4
#define MDIO_CMD_MII_PHY_ADDR_MASK 0x0001f000
#define MDIO_CMD_MII_PHY_ADDR_SHIFT 12

bool Read(uint32_t address, uint32_t nRegister, uint16_t& nValue)
{
    DEBUG_ENTRY();

    uint32_t nCmd = ((0x03 & 0x07) << 20);
    nCmd |= ((address << MDIO_CMD_MII_PHY_ADDR_SHIFT) & MDIO_CMD_MII_PHY_ADDR_MASK);
    nCmd |= ((nRegister << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) & 0x000007F0);
    nCmd |= MDIO_CMD_MII_BUSY;

    auto bResult = false;
    auto micros = H3_TIMER->AVS_CNT1;

    while (H3_TIMER->AVS_CNT1 - micros < CONFIG_MDIO_TIMEOUT)
    {
        if (!(H3_EMAC->MII_CMD & MDIO_CMD_MII_BUSY))
        {
            bResult = true;
            break;
        }
    };

    if (!bResult)
    {
        DEBUG_EXIT();
        return false;
    }

    H3_EMAC->MII_CMD = nCmd;

    bResult = false;
    micros = H3_TIMER->AVS_CNT1;

    while (H3_TIMER->AVS_CNT1 - micros < CONFIG_MDIO_TIMEOUT)
    {
        if (!(H3_EMAC->MII_CMD & MDIO_CMD_MII_BUSY))
        {
            bResult = true;
            break;
        }
    };

    if (!bResult)
    {
        DEBUG_EXIT();
        return false;
    }

    nValue = static_cast<uint16_t>(H3_EMAC->MII_DATA);

    DEBUG_PRINTF("%.2x %.2x %.4x", address, nRegister, nValue);
    DEBUG_EXIT();
    return true;
}

bool Write(uint32_t address, uint32_t nRegister, uint16_t nValue)
{
    uint32_t miiaddr = (nRegister << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) & MDIO_CMD_MII_PHY_REG_ADDR_MASK;
    miiaddr |= (address << MDIO_CMD_MII_PHY_ADDR_SHIFT) & MDIO_CMD_MII_PHY_ADDR_MASK;

    miiaddr |= MDIO_CMD_MII_WRITE;
    miiaddr |= MDIO_CMD_MII_BUSY;

    H3_EMAC->MII_DATA = nValue;
    H3_EMAC->MII_CMD = miiaddr;

    auto bResult = false;
    auto micros = H3_TIMER->AVS_CNT1;

    while (H3_TIMER->AVS_CNT1 - micros < CONFIG_MDIO_TIMEOUT)
    {
        if (!(H3_EMAC->MII_CMD & MDIO_CMD_MII_BUSY))
        {
            bResult = true;
            break;
        }
    };

    //	DEBUG_PRINTF("%d %.2x %.2x %.4x", bResult, address, nRegister, nValue);
    return bResult;
}

bool Config([[maybe_unused]] const uint32_t address)
{
    DEBUG_ENTRY();

    /**
     * We are starting from U-Boot which is setting the PHY. Resetting the PHY is not
     * needed here. It gives issues ....
     */
#if 0
	if (!phy::Write(address, mmi::REG_BMCR, mmi::BMCR_RESET)) {
		DEBUG_PUTS("PHY reset failed");
		return false;
	}

	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */

	const auto nMilllis = H3_TIMER->AVS_CNT0;
	uint16_t nValue;

	while (H3_TIMER->AVS_CNT0 - nMilllis < 500) {
		if (!PhyRead(address, mmi::REG_BMCR, nValue)) {
			DEBUG_PUTS("PHY status read failed");
			return false;
		}

		if (!(nValue & mmi::BMCR_RESET)) {
			DEBUG_PRINTF("%u", H3_TIMER->AVS_CNT0 - nMilllis);
			DEBUG_EXIT();
			return true;
		}
	}

	if (nValue & mmi::BMCR_RESET) {
		DEBUG_PUTS("PHY reset timed out");
		return false;
	}
#endif
    DEBUG_EXIT();
    return true;
}

} // namespace net::phy
