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
#include "emac/mmi.h"
#include "firmware/debug/debug_printbits.h"
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

void CustomizedStatus(phy::Status& phy_status)
{
    uint16_t value;
    phy::Read(PHY_ADDRESS, mmi::REG_BMSR, value);

    debug::PrintBits(value);

    phy_status.duplex = Duplex::kDuplexFull;
    phy_status.speed = Speed::kSpeed100;
    phy_status.link = (value & mmi::BMSR_LINKED_STATUS) ? Link::kStateUp : Link::kStateDown;
    phy_status.autonegotiation = (value & mmi::BMSR_AUTONEGO_COMPLETE);
}
} // namespace net::phy
