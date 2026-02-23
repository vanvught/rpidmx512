/**
 * @file link_check.cpp
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

#include "emac/net_link_check.h"
#include "emac/phy.h"
#include "emac/mmi.h"
#include "firmware/debug/debug_debug.h"

#define PHY_REG_MICR 0x11U
#define PHY_REG_MISR 0x12U
#define PHY_INT_AND_OUTPUT_ENABLE 0x03U
#define PHY_LINK_INT_ENABLE 0x20U

#if !defined(PHY_ADDRESS)
#define PHY_ADDRESS 1
#endif

namespace net::link
{
#if defined(ENET_LINK_CHECK_USE_INT) || defined(ENET_LINK_CHECK_USE_PIN_POLL)
void PinEnable()
{
    uint16_t phy_value = PHY_INT_AND_OUTPUT_ENABLE;
    phy::Write(PHY_ADDRESS, PHY_REG_MICR, phy_value);

    phy::Read(PHY_ADDRESS, PHY_REG_MICR, phy_value);

    if (PHY_INT_AND_OUTPUT_ENABLE != phy_value)
    {
        DEBUG_PUTS("PHY_INT_AND_OUTPUT_ENABLE != phy_value");
    }

    phy_value = PHY_LINK_INT_ENABLE;
    phy::Write(PHY_ADDRESS, PHY_REG_MISR, phy_value);
}

void PinRecovery()
{
    uint16_t phy_value;
    phy::Read(PHY_ADDRESS, PHY_REG_MISR, phy_value);
    phy::Read(PHY_ADDRESS, mmi::REG_BMSR, phy_value);
}
#endif
} // namespace net::link
