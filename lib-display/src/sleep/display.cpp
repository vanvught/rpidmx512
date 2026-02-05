/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_DISPLAY)
#undef NDEBUG
#endif

#include "display.h"
#include "softwaretimers.h"
#include "firmware/debug/debug_debug.h"

static TimerHandle_t s_timer_id = kTimerIdNone;

static void SleepTimer([[maybe_unused]] TimerHandle_t handle)
{
    Display::Get()->SetSleep(true);
    SoftwareTimerDelete(s_timer_id);
}

void Display::SetSleepTimer(bool active)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("active=%d, sleep_timeout_=%u, s_timer_id=%d", active, sleep_timeout_, s_timer_id);

    if (!active)
    {
        SoftwareTimerDelete(s_timer_id);
        DEBUG_EXIT();
        return;
    }

    if (s_timer_id == kTimerIdNone)
    {
        s_timer_id = SoftwareTimerAdd(sleep_timeout_, SleepTimer);
        DEBUG_EXIT();
        return;
    }

    SoftwareTimerChange(s_timer_id, sleep_timeout_);
    DEBUG_EXIT();
}
