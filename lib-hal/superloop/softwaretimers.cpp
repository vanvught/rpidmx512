/**
 * @file softwaretimers.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# if !defined (CONFIG_REMOTECONFIG_MINIMUM)
#  pragma GCC push_options
#  pragma GCC optimize ("O2")
#  pragma GCC optimize ("no-tree-loop-distribute-patterns")
# endif
#endif

#include <cstdint>

#include "superloop/softwaretimers.h"
#include "hardware.h"

void console_error(const char *);

struct Timer {
	uint32_t nExpireTime;
	uint32_t nIntervalMillis;
	int32_t nId;
	TimerCallbackFunction_t pCallbackFunction;
};

static Timer m_Timers[hal::SOFTWARE_TIMERS_MAX];
static uint32_t m_nTimersCount;
static int32_t m_nNextId;

TimerHandle_t SoftwareTimerAdd(const uint32_t nIntervalMillis, const TimerCallbackFunction_t pCallbackFunction) {
	if (m_nTimersCount >= hal::SOFTWARE_TIMERS_MAX) {
#ifndef NDEBUG
		console_error("SoftwareTimerAdd: Max timer limit reached\n");
#endif
		return -1;
	}

	const auto nCurrentTime = Hardware::Get()->Millis();
	// TODO Prevent potential overflow when calculating expiration time.

	Timer newTimer = {
			.nExpireTime = nCurrentTime + nIntervalMillis,
			.nIntervalMillis = nIntervalMillis,
			.nId = m_nNextId++,
			.pCallbackFunction = pCallbackFunction,
	};

	m_Timers[m_nTimersCount++] = newTimer;

	return newTimer.nId;
}

bool SoftwareTimerDelete(TimerHandle_t& nId) {
	for (uint32_t i = 0; i < m_nTimersCount; ++i) {
		if (m_Timers[i].nId == nId) {
			// Swap with the last timer to efficiently remove the current timer
			m_Timers[i] = m_Timers[m_nTimersCount - 1];
			--m_nTimersCount;
			nId = -1;
			return true;
		}
	}

#ifndef NDEBUG
    console_error("SoftwareTimerDelete: Timer not found\n");
#endif

	return false;
}

bool SoftwareTimerChange(const TimerHandle_t nId, const uint32_t nIntervalMillis) {
	for (uint32_t i = 0; i < m_nTimersCount; ++i) {
		if (m_Timers[i].nId == nId) {
			const auto nCurrentTime = Hardware::Get()->Millis();
			m_Timers[i].nExpireTime = nCurrentTime + nIntervalMillis;
			m_Timers[i].nIntervalMillis = nIntervalMillis;
			return true;
		}
	}

#ifndef NDEBUG
    console_error("SoftwareTimerChange: Timer not found\n");
#endif

	return false;
}

void SoftwareTimerRun() {
    const auto nCurrentTime = Hardware::Get()->Millis();

    for (uint32_t i = 0; i < m_nTimersCount; i++) {
        if (m_Timers[i].nExpireTime <= nCurrentTime) {
        	m_Timers[i].pCallbackFunction(m_Timers[i].nId);
            m_Timers[i].nExpireTime = nCurrentTime + m_Timers[i].nIntervalMillis;
        }
    }
}
