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

#if defined (DEBUG_HAL)
# undef NDEBUG
#endif

#include <cstdint>

#include "hal.h"
#include "hal_statusled.h"

#include "softwaretimers.h"

#include "debug.h"

void h3_status_led_set(int);

static int32_t m_nTimerId = -1;
static int32_t m_nToggleLed;

static void ledblink([[maybe_unused]] TimerHandle_t nHandle) {
	m_nToggleLed ^= 0x1;
	h3_status_led_set(m_nToggleLed);
}

namespace hal {
void statusled_set_frequency(const uint32_t nFrequencyHz) {
	if (nFrequencyHz == 0) {
		SoftwareTimerDelete (m_nTimerId);
		h3_status_led_set(0);
		return;
	}

	if (nFrequencyHz == 255) {
		SoftwareTimerDelete (m_nTimerId);
		h3_status_led_set(1);
		return;
	}

	if (m_nTimerId < 0) {
		m_nTimerId = SoftwareTimerAdd((1000U / nFrequencyHz), ledblink);
		DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
		return;
	}

	DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
	SoftwareTimerChange(m_nTimerId, (1000U / nFrequencyHz));
}
}  // namespace hal
