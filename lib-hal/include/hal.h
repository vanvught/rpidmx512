/**
 * @file hal.h
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

#ifndef HAL_H_
#define HAL_H_

#include <cstdint>

namespace hal
{
enum class BootDevice
{
    UNKOWN,
    FEL,
    MMC0,
    SPI,
    HDD,
    FLASH,
    RAM
};

BootDevice GetBootDevice();

void Init();

uint32_t Uptime();

float CoreTemperatureCurrent();

bool Reboot();
void RebootHandler();
} // namespace hal

#if defined(__linux__) || defined(__APPLE__)
#if defined(CONFIG_HAL_USE_MINIMUM)
#include "linux/minimum/hal.h"
#else
#include "linux/hal.h"
#endif
#else
#if defined(H3)
#include "h3/hal.h"
#elif defined(GD32)
#include "gd32/hal.h"
#else
#include "rpi/hal.h"
#endif
#endif

#endif  // HAL_H_
