/**
 * @file softwaretimers.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SOFTWARETIMERS_H_
#define SOFTWARETIMERS_H_

#include <cstdint>

namespace hal
{
static constexpr uint32_t kSoftwareTimersMax =
#if defined(CONFIG_HAL_TIMERS_COUNT)
    CONFIG_HAL_TIMERS_COUNT;
#else
    12;
#endif
} // namespace hal

#if defined(USE_FREE_RTOS)
#ifdef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include "../FreeRTOS/FreeRTOS-Kernel/include/FreeRTOS.h"
#include "../FreeRTOS/FreeRTOS-Kernel/include/timers.h"
#define kTimerIdNone nullptr
inline TimerHandle_t SoftwareTimerAdd(const uint32_t nIntervalMillis, const TimerCallbackFunction_t pCallbackFunction)
{
    const auto xTimer = xTimerCreate("", nIntervalMillis / portTICK_PERIOD_MS, pdTRUE, nullptr, pCallbackFunction);
    if (xTimer != nullptr) xTimerStart(xTimer, 0);
    return xTimer;
}

inline bool SoftwareTimerDelete(TimerHandle_t& nId)
{
    TimerHandle_t t = nId;
    const auto b = (xTimerDelete(t, 0) != pdFAIL);
    if (b) nId = kTimerIdNone;
    return b;
}

inline bool SoftwareTimerChange(const TimerHandle_t nId, const uint32_t nIntervalMillis)
{
    return xTimerChangePeriod(nId, nIntervalMillis / portTICK_PERIOD_MS, 0) != pdFAIL;
}
#ifdef __cplusplus
#pragma GCC diagnostic pop
#endif
#else
#include "superloop/softwaretimers.h"
static constexpr TimerHandle_t kTimerIdNone = -1;
#endif

#endif  // SOFTWARETIMERS_H_
