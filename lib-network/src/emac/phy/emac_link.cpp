/**
 * net_link::check.cpp
 *
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

#include <cstdint>

#include "core/netif.h"
#include "emac/emac_link_check.h"
#include "emac/emac_phy.h"
#include "emac/emac.h"
#include "hal_watchdog.h" // IWYU pragma: keep
#include "firmware/debug/debug_debug.h"

static constexpr uint16_t kAddress =
#if !defined(PHY_ADDRESS)
    1;
#else
    PHY_ADDRESS;
#endif

namespace emac::link {
#if defined(ENET_LINK_CHECK_USE_INT)
void InterruptInit() {
    link::PinEnable();
    link::PinRecovery();
    link::GpioInit();
    link::ExtiInit();
}
#endif

#if defined(ENET_LINK_CHECK_USE_PIN_POLL)
void PinPollInit() {
    link::PinEnable();
    link::PinRecovery();
    link::GpioInit();
}
#endif

emac::phy::Link StatusRead() {
    return emac::phy::GetLink(kAddress);
}

void HandleChange(emac::phy::Link state) {
    DEBUG_PRINTF("emac::phy::Link %s", state == emac::phy::Link::kStateUp ? "UP" : "DOWN");

    if (phy::Link::kStateUp == state) {
        const auto kIsWatchdog = hal::Watchdog();

        if (kIsWatchdog) {
            hal::WatchdogStop();
        }

        phy::Status phy_status;
        phy::Start(kAddress, phy_status);

        emac::AdjustLink(phy_status);

        if (kIsWatchdog) {
            hal::WatchdogInit();
        }

        netif::SetLinkUp();
        return;
    }

    netif::SetLinkDown();
}
} // namespace emac::link
