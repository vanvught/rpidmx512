/**
 * @file link_handle_change.cpp
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_PHY)
#undef NDEBUG
#endif

#include "hal_watchdog.h"
#include "emac/emac.h"
#include "emac/phy.h"
#include "core/netif.h"
#include "firmware/debug/debug_debug.h"

#if !defined(PHY_ADDRESS)
#define PHY_ADDRESS 1
#endif

namespace net::link
{
void HandleChange(net::phy::Link state)
{
    DEBUG_PRINTF("net::phy::Link %s", state == net::phy::Link::kStateUp ? "UP" : "DOWN");

    if (phy::Link::kStateUp == state)
    {
        const auto kIsWatchdog = hal::Watchdog();

        if (kIsWatchdog)
        {
            hal::WatchdogStop();
        }

        phy::Status phy_status;
        phy::Start(PHY_ADDRESS, phy_status);

        net::emac::AdjustLink(phy_status);

        if (kIsWatchdog)
        {
            hal::WatchdogInit();
        }

        netif::SetLinkUp();
        return;
    }

    netif::SetLinkDown();
}
} // namespace net::link
