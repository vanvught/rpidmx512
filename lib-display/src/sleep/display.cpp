/**
 * @file display.cpp
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

#if defined (DEBUG_DISPLAY)
# undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "display.h"

#include "softwaretimers.h"

#include "debug.h"

static TimerHandle_t s_nTimerId = TIMER_ID_NONE;

static void sleep_timer([[maybe_unused]] TimerHandle_t nHandle) {
	Display::Get()->SetSleep(true);
	SoftwareTimerDelete(s_nTimerId);
}

void Display::SetSleepTimer(const bool bActive) {
	DEBUG_ENTRY
	DEBUG_PRINTF("bActive=%d, m_nSleepTimeout=%u, s_nTimerId=%d", bActive, m_nSleepTimeout, s_nTimerId);

	if (!bActive) {
		SoftwareTimerDelete(s_nTimerId);
		DEBUG_EXIT
		return;
	}

	if (s_nTimerId == TIMER_ID_NONE) {
		s_nTimerId = SoftwareTimerAdd(m_nSleepTimeout, sleep_timer);
		DEBUG_EXIT
		return;
	}

	SoftwareTimerChange(s_nTimerId, m_nSleepTimeout);
	DEBUG_EXIT
}
