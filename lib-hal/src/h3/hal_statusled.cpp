/**
 * @file hal_statusled.cpp
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

#include <cstdint>

#include "hal.h"
#include "hal_statusled.h"

#include "softwaretimers.h"

 #include "firmware/debug/debug_debug.h"

void h3_status_led_set(int);

static int32_t s_timer_id = -1;
static int32_t s_toggle_led;

static void Ledblink([[maybe_unused]] TimerHandle_t handle)
{
    s_toggle_led ^= 0x1;
    h3_status_led_set(s_toggle_led);
}

namespace hal::statusled
{
void SetFrequency(uint32_t frequency_hz)
{
    if (frequency_hz == 0)
    {
        SoftwareTimerDelete(s_timer_id);
        h3_status_led_set(0);
        return;
    }

    if (frequency_hz == 255)
    {
        SoftwareTimerDelete(s_timer_id);
        h3_status_led_set(1);
        return;
    }

    if (s_timer_id < 0)
    {
        s_timer_id = SoftwareTimerAdd((1000U / frequency_hz), Ledblink);
        DEBUG_PRINTF("m_nTimerId=%d", s_timer_id);
        return;
    }

    DEBUG_PRINTF("m_nTimerId=%d", s_timer_id);
    SoftwareTimerChange(s_timer_id, (1000U / frequency_hz));
}
} // namespace hal::statusled
