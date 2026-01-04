/**
 * @file hal_reboot.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_HAL)
#undef NDEBUG
#endif

#include <cstdio>

#include "h3_watchdog.h"
#include "h3_gpio.h"
#include "h3_board.h"
#include "arm/arm.h"
#include "arm/synchronize.h"
#include "hal.h"
#include "hal_statusled.h"

#if !defined(DISABLE_RTC)
#include "hwclock.h"
#endif

#include "configstore.h"

#if !defined(NO_EMAC)
namespace net
{
void Shutdown();
} // namespace net
#endif

namespace hal
{
bool g_bWatchdog;

bool Reboot()
{
    puts("Rebooting ...");

    h3_watchdog_disable();

    ConfigstoreCommit();
#if !defined(DISABLE_RTC)
    HwClock::Get()->SysToHc();
#endif
    RebootHandler();
#if !defined(NO_EMAC)
    net::Shutdown();
#endif
    clean_data_cache();
    invalidate_data_cache();

    H3GpioFsel(EXT_SPI_MOSI, GPIO_FSEL_INPUT);
    H3GpioSetPud(EXT_SPI_MOSI, GPIO_PULL_DOWN);
    H3GpioFsel(EXT_SPI_CLK, GPIO_FSEL_INPUT);
    H3GpioSetPud(EXT_SPI_CLK, GPIO_PULL_DOWN);
    H3GpioFsel(EXT_SPI_CS, GPIO_FSEL_INPUT);
    H3GpioSetPud(EXT_SPI_CS, GPIO_PULL_DOWN);

    hal::statusled::SetMode(hal::statusled::Mode::REBOOT);

    h3_watchdog_enable();

    for (;;)
    {
        Run();
    }

    __builtin_unreachable();
    return true;
}
} // namespace hal
