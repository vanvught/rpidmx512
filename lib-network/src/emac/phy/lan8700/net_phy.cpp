/**
 * net_phy.cpp
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

namespace net {
void phy_customized_led() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void phy_customized_timing() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

/**
 * PHY SPECIAL CONTROL/STATUS REGISTER Index (In Decimal): 31
 * @param phyStatus
 */
void phy_customized_status(PhyStatus& phyStatus) {
	phyStatus.link = link_status_read();

	uint16_t nValue;
	phy_read(PHY_ADDRESS, 0x1f, nValue);

	phyStatus.duplex = ((nValue & BIT(4)) == BIT(4)) ? Duplex::DUPLEX_FULL : Duplex::DUPLEX_HALF;
	phyStatus.speed = ((nValue & BIT(2)) == BIT(2)) ? Speed::SPEED10 : Speed::SPEED100;
	phyStatus.bAutonegotiation = ((nValue & BIT(12)) == BIT(12));
}

}  // namespace net
