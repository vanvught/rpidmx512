/**
 * @file flashcodeinstall.cpp
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cassert>

#include "flashcodeinstall.h"
#include "firmware.h"
#include "display.h"
#include "hal_watchdog.h"
 #include "firmware/debug/debug_debug.h"

bool FlashCodeInstall::WriteFirmware(const uint8_t* buffer, uint32_t size)
{
    DEBUG_ENTRY();

    assert(buffer != nullptr);
    assert(size != 0);

    DEBUG_PRINTF("(%p + %p)=%p, flash_size_=%u", OFFSET_UIMAGE, size, (OFFSET_UIMAGE + size), static_cast<unsigned int>(flash_size_));

    if ((OFFSET_UIMAGE + size) > flash_size_)
    {
        printf("Error: (OFFSET_UIMAGE + size) %u > flash_size_ %u\n", static_cast<unsigned int>(OFFSET_UIMAGE + size), static_cast<unsigned int>(flash_size_));
        DEBUG_EXIT();
        return false;
    }

    const auto kWatchdog = hal::Watchdog();

    if (kWatchdog)
    {
        hal::WatchdogStop();
    }

    puts("Write firmware");

    const auto kSectorSize = FlashCode::GetSectorSize();
    const auto kEraseSize = (size + kSectorSize - 1) & ~(kSectorSize - 1);

    DEBUG_PRINTF("size=%x, kSectorSize=%x, kEraseSize=%x", size, kSectorSize, kEraseSize);

    Display::Get()->TextStatus("Erase", console::Colours::kConsoleGreen);

    flashcode::Result result;

    while (!FlashCode::Erase(OFFSET_UIMAGE, kEraseSize, result));

    if (flashcode::Result::kError == result)
    {
        puts("Error: flash erase");
        return false;
    }

    Display::Get()->TextStatus("Writing", console::Colours::kConsoleGreen);

    while (!FlashCode::Write(OFFSET_UIMAGE, size, buffer, result));

    if (flashcode::Result::kError == result)
    {
        puts("Error: flash write");
        return false;
    }

    if (kWatchdog)
    {
        hal::WatchdogInit();
    }

    Display::Get()->TextStatus("Done", console::Colours::kConsoleGreen);

    DEBUG_EXIT();
    return true;
}
