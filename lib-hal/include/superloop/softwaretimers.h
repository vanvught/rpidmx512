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

#ifndef HAL_SUPERLOOP_TIMERS_H_
#define HAL_SUPERLOOP_TIMERS_H_

#include <cstdint>

typedef int32_t TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t);

TimerHandle_t SoftwareTimerAdd(const uint32_t nIntervalMillis, const TimerCallbackFunction_t callback);
bool SoftwareTimerDelete(TimerHandle_t& nId);
bool SoftwareTimerChange(const TimerHandle_t nId, const uint32_t nIntervalMillis);

void SoftwareTimerRun();

#endif /* HAL_SUPERLOOP_TIMERS_H_ */
