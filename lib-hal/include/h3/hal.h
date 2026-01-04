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

#ifndef H3_HAL_H_
#define H3_HAL_H_

#include <cstdint>

#include "superloop/softwaretimers.h"

#if defined(DEBUG_STACK)
void stack_debug_run();
#endif

namespace hal
{
#if defined(ORANGE_PI)
static constexpr uint32_t kBoardId = 0;
#elif defined(ORANGE_PI_ONE)
static constexpr uint32_t kBoardId = 1;
#else
#error Platform not supported
#endif
static constexpr uint32_t kReleaseId = 0;
static constexpr const char kWebsite[] = "www.orangepi-dmx.org";
static constexpr float kCoreTemperatureMin = -40.0;
static constexpr float kCoreTemperatureMax = +90.0;

inline void Run()
{
    SoftwareTimerRun();
#if defined(DEBUG_STACK)
    stack_debug_run();
#endif
}
} // namespace hal

#endif  // H3_HAL_H_
