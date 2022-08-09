/**
 * @file ledblink.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_LEDBLINK_H_
#define GD32_LEDBLINK_H_

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "gd32_bitbanging595.h"
#include "panel_led.h"

#include "debug.h"

extern volatile uint32_t s_nSysTickMillis;

class LedBlink {
public:
	LedBlink();

	void SetFrequency(const uint32_t nFreqHz) {
		m_nFreqHz = nFreqHz;

		switch (nFreqHz) {
		case 0:
			m_nTicksPerSecond = 0;
#if defined (USE_LEDBLINK_BITBANGING595)
			hal::panel_led_off(hal::panelled::ACTIVITY);
#else
			GPIO_BC(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
			break;
		case 1:
			m_nTicksPerSecond = (1000 / 1);
			break;
		case 3:
			m_nTicksPerSecond = (1000 / 3);
			break;
		case 5:
			m_nTicksPerSecond = (1000 / 5);
			break;
		case 255:
			m_nTicksPerSecond = 0;
#if defined (USE_LEDBLINK_BITBANGING595)
			hal::panel_led_on(hal::panelled::ACTIVITY);
#else
			GPIO_BOP(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
			break;
		default:
			m_nTicksPerSecond = (1000 / nFreqHz);
			break;
		}
	}

	uint32_t GetFrequency() const {
		return m_nFreqHz;
	}

	void SetMode(ledblink::Mode tMode);

	ledblink::Mode GetMode() const {
		return m_tMode;
	}

	void Run() {
		if (__builtin_expect (m_nTicksPerSecond != 0, 1)) {
			if (__builtin_expect (!(s_nSysTickMillis - m_nMillisPrevious < m_nTicksPerSecond), 1)) {
				m_nMillisPrevious = s_nSysTickMillis;

				m_nToggleLed ^= 0x1;

				if (m_nToggleLed != 0) {
#if defined (USE_LEDBLINK_BITBANGING595)
					hal::panel_led_on(hal::panelled::ACTIVITY);
#else
					GPIO_BOP(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
				} else {
#if defined (USE_LEDBLINK_BITBANGING595)
					hal::panel_led_off(hal::panelled::ACTIVITY);
#else
					GPIO_BC(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
				}
			}
		}

#if defined (USE_LEDBLINK_BITBANGING595)
		bitBanging595.Run();
#endif
	}

	void SetLedBlinkDisplay(LedBlinkDisplay *pLedBlinkDisplay) {
		m_pLedBlinkDisplay = pLedBlinkDisplay;
	}

	static LedBlink* Get() {
		return s_pThis;
	}

private:
	void PlatformInit() {
#if !defined(USE_LEDBLINK_BITBANGING595)
		rcu_periph_clock_enable(LED_BLINK_GPIO_CLK);
# if !defined (GD32F4XX)
		gpio_init(LED_BLINK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_BLINK_PIN);
# else
		gpio_mode_set(LED_BLINK_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BLINK_PIN);
		gpio_output_options_set(LED_BLINK_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_BLINK_PIN);
# endif
		GPIO_BC(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
	}

private:
#if defined (USE_LEDBLINK_BITBANGING595)
	BitBanging595 bitBanging595;
#endif
	uint32_t m_nFreqHz { 0 };
	ledblink::Mode m_tMode { ledblink::Mode::UNKNOWN };
	LedBlinkDisplay *m_pLedBlinkDisplay { nullptr };
	//
	uint32_t m_nTicksPerSecond { 1000 / 2 };
	int32_t m_nToggleLed { 0 };
	uint32_t m_nMillisPrevious { 0 };

	static LedBlink *s_pThis;
};

#endif /* GD32_LEDBLINK_H_ */
