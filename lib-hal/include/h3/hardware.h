/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_HARDWARE_H_
#define H3_HARDWARE_H_

#include <cstdint>
#include <time.h>

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "h3.h"
#include "superloop/softwaretimers.h"

#include "debug.h"

void h3_status_led_set(int);

class Hardware {
public:
	Hardware();

	void SetModeWithLock(hardware::ledblink::Mode mode, bool doLock);
	void SetMode(hardware::ledblink::Mode mode);
	hardware::ledblink::Mode GetMode() const {
		return m_Mode;
	}

	static Hardware *Get() {
		return s_pThis;
	}

private:
	static void ledblink([[maybe_unused]] TimerHandle_t nHandle) {
		m_nToggleLed ^= 0x1;
		h3_status_led_set(m_nToggleLed);
	}

	void SetFrequency(const uint32_t nFreqHz) {
		if (nFreqHz == 0) {
			SoftwareTimerDelete(m_nTimerId);
			h3_status_led_set(0);
			return;
		}

		if (nFreqHz == 255) {
			SoftwareTimerDelete(m_nTimerId);
			h3_status_led_set(1);
			return;
		}

		if (m_nTimerId < 0) {
			m_nTimerId = SoftwareTimerAdd((1000U / nFreqHz),ledblink);
			DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
			return;
		}

		DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
		SoftwareTimerChange(m_nTimerId, (1000U / nFreqHz));
	}

private:
	hardware::ledblink::Mode m_Mode { hardware::ledblink::Mode::UNKNOWN };
	bool m_doLock { false };
	int32_t m_nTimerId { -1 };

	static inline int32_t m_nToggleLed { 0 };
	static inline Hardware *s_pThis;
};

#endif /* H3_HARDWARE_H_ */
