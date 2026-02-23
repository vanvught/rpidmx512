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

#include "emac/phy.h"
#include "emac/net_link_check.h"
#include "emac/mmi.h"

#include "firmware/debug/debug_debug.h"

#if !defined(BIT)
#define BIT(x) static_cast<uint16_t>(1U << (x))
#endif

#if !defined(PHY_ADDRESS)
#define PHY_ADDRESS 1
#endif

namespace net::phy
{
void CustomizedLed()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void CustomizedTiming()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

/**
 * PHY Status Register (PHYSTS), address 10h
 * @param phyStatus
 */
void CustomizedStatus(phy::Status& phy_status)
{
    uint16_t value;
    phy::Read(PHY_ADDRESS, 0x10, value);

    phy_status.link = ((value & BIT(0)) == BIT(0)) ? phy::Link::kStateUp : phy::Link::kStateDown;
    phy_status.duplex = ((value & BIT(2)) == BIT(2)) ? phy::Duplex::kDuplexFull : phy::Duplex::kDuplexHalf;
    phy_status.speed = ((value & BIT(1)) == BIT(1)) ? phy::Speed::kSpeed10 : phy::Speed::kSpeed100;
    phy_status.autonegotiation = ((value & BIT(4)) == BIT(4));
}
} // namespace net::phy
