/**
 * @file softwaretimers.cpp
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

#if defined(DEBUG_HAL_TIMERS)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-funroll-loops")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif
#endif

#include <cstdint>

#include "softwaretimers.h"
#include "hal_millis.h"
#include "firmware/debug/debug_debug.h"

namespace console
{
void Error(const char*);
} // namespace console

struct Timer
{
    uint32_t expire_time;                      ///< Absolute expire time in milliseconds (wrap-around safe).
    uint32_t interval_millis;                  ///< Period in milliseconds
    int32_t id;                                ///< Opaque handle returned to the caller.
    TimerCallbackFunction_t callback_function; ///< Callback invoked on expiry; must be non-null.
};

static Timer s_timers[hal::kSoftwareTimersMax]; ///< Timer storage pool.
static uint32_t s_timers_count = 0;             ///< Number of active timers (0..hal::SOFTWARE_TIMERS_MAX).
static int32_t s_next_id = 0;                   ///< Monotonically increasing ID source (may wrap, see notes).
static uint32_t s_timer_current = 0;            ///< bRound-robin cursor for SoftwareTimerRun().

/**
 * @brief Create and start a periodic (or one-shot) software timer.
 *
 * @param interval_millis  Period in milliseconds. If set to 0, the timer is treated as one-shot.
 * @param kCallbackFunction Callback function. Must be non-null.
 * @return TimerHandle_t    A non-negative handle on success; -1 on failure (pool full or bad args).
 *
 * @warning Callbacks run in the context that calls SoftwareTimerRun(). Keep them short,
 *          non-blocking, and ISR-safe *only if* SoftwareTimerRun() is called from an ISR.
 * @note    The first expiration is scheduled relative to the current @ref hal::Millis().
 */
TimerHandle_t SoftwareTimerAdd(uint32_t interval_millis, const TimerCallbackFunction_t kCallbackFunction)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_timers_count=%u", s_timers_count);

    if (s_timers_count >= hal::kSoftwareTimersMax)
    {
#ifndef NDEBUG
        console::Error("SoftwareTimerAdd: Max timer limit reached");
#endif
        return -1;
    }

    const auto kCurrentTime = hal::Millis();
    // TODO (a) Prevent potential overflow when calculating expiration time.

    Timer new_timer = {
        .expire_time = kCurrentTime + interval_millis,
        .interval_millis = interval_millis,
        .id = s_next_id++,
        .callback_function = kCallbackFunction,
    };

    s_timers[s_timers_count++] = new_timer;

    DEBUG_EXIT();
    return new_timer.id;
}

/**
 * @brief Delete a timer.
 *
 * @param id [in,out] Handle to delete. Set to -1 on success.
 * @return true  If a timer with the given handle was found and removed.
 * @return false Otherwise.
 *
 * @note Deletion is O(1): the removed slot is replaced with the last active timer.
 *       This changes the order of timers.
 */
bool SoftwareTimerDelete(TimerHandle_t& id)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_timers_count=%u", s_timers_count);

    for (uint32_t i = 0; i < s_timers_count; ++i)
    {
        if (s_timers[i].id == id)
        {
            // Swap with the last timer to efficiently remove the current timer
            s_timers[i] = s_timers[s_timers_count - 1];
            --s_timers_count;
            // Keep the round-robin cursor within bounds.
            if (s_timer_current >= s_timers_count)
            {
                s_timer_current = 0;
            }

            id = -1;

            DEBUG_ENTRY();
            return true;
        }
    }

#ifndef NDEBUG
    console::Error("SoftwareTimerDelete: Timer not found");
#endif

    DEBUG_EXIT();
    return false;
}

/**
 * @brief Change a timer’s period and restart its countdown from now.
 *
 * @param id              Timer handle.
 * @param interval_millis New period in milliseconds (0 => one-shot).
 * @return true  On success.
 * @return false If the handle was not found.
 */
bool SoftwareTimerChange(TimerHandle_t id, uint32_t interval_millis)
{
    for (uint32_t i = 0; i < s_timers_count; ++i)
    {
        if (s_timers[i].id == id)
        {
            const auto kCurrentTime = hal::Millis();
            s_timers[i].expire_time = kCurrentTime + interval_millis;
            s_timers[i].interval_millis = interval_millis;
            return true;
        }
    }

#ifndef NDEBUG
    console::Error("SoftwareTimerChange: Timer not found");
#endif

    return false;
}

/**
 * @brief Service one timer slot.
 *
 * If the current slot has expired, its callback is invoked exactly once. The
 * timer is then rescheduled if it is periodic.
 *
 * @note This function is intentionally O(1) per invocation.
 * @note If callbacks delete or add timers, this remains safe due to the
 *       swap-delete approach and the post-callback cursor normalization.
 */
void SoftwareTimerRun()
{
    if (s_timers_count == 0) [[unlikely]]
    {
        return;
    }

    const uint32_t kNow = hal::Millis();
    Timer& t = s_timers[s_timer_current];

    if (static_cast<int32_t>(kNow - t.expire_time) >= 0) [[unlikely]]
    {
        const int32_t kId = t.id;
        const uint32_t kInterval = t.interval_millis;
        auto cb = t.callback_function;

        cb(kId);

        // reschedule from NOW to avoid pile-ups after delays
        s_timers[s_timer_current].expire_time = kNow + kInterval;
    }

    // Advance round-robin cursor (bounded next call).
    ++s_timer_current;
    if (s_timer_current >= s_timers_count)
    {
        s_timer_current = 0;
    }
}
