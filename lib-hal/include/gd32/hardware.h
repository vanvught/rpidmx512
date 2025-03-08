/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_HARDWARE_H_
#define GD32_HARDWARE_H_

#include <cstdint>
#include <cstring>
#include <time.h>

#include "hwclock.h"

#include "gd32.h"
#include "gd32_adc.h"
#include "gd32_millis.h"

#if !defined (CONFIG_LEDBLINK_USE_PANELLED) && (defined (GD32F4XX) || defined(GD32H7XX))
# define HAL_HAVE_PORT_BIT_TOGGLE
#endif

#if defined (ENABLE_USB_HOST) && defined (CONFIG_USB_HOST_MSC)
extern "C" {
#include "usbh_core.h"
#if defined (GD32H7XX) || defined (GD32F4XX)
 extern usbh_host usb_host_msc;
#else
 extern usbh_host usb_host;
#endif
}
#endif

#if defined (DEBUG_STACK)
 void stack_debug_run();
#endif
#if defined (DEBUG_EMAC)
 void emac_debug_run();
#endif

#ifdef NDEBUG
 void console_error(const char *);
#endif

#include "softwaretimers.h"

#include "panel_led.h"

#include "debug.h"

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
#if defined(HAL_HAVE_PORT_BIT_TOGGLE)
		GPIO_TG(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#else
		m_nToggleLed = -m_nToggleLed;

		if (m_nToggleLed > 0) {
# if defined (CONFIG_LEDBLINK_USE_PANELLED)
			hal::panel_led_on(hal::panelled::ACTIVITY);
# else
			GPIO_BOP(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
# endif
		} else {
# if defined (CONFIG_LEDBLINK_USE_PANELLED)
			hal::panel_led_off(hal::panelled::ACTIVITY);
# else
			GPIO_BC(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
# endif
		}
#endif
	}

	void SetFrequency(const uint32_t nFreqHz) {
		DEBUG_ENTRY
		DEBUG_PRINTF("m_nTimerId=%d, nFreqHz=%u", m_nTimerId, nFreqHz);

		if (m_nTimerId == TIMER_ID_NONE) {
			m_nTimerId = SoftwareTimerAdd((1000U / nFreqHz), ledblink);
			DEBUG_EXIT
			return;
		}

		switch (nFreqHz) {
		case 0:
			SoftwareTimerDelete(m_nTimerId);
#if defined (CONFIG_LEDBLINK_USE_PANELLED)
			hal::panel_led_off(hal::panelled::ACTIVITY);
#else
			GPIO_BC(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
			break;
# if !defined (CONFIG_HAL_USE_MINIMUM)
		case 1:
			SoftwareTimerChange(m_nTimerId, (1000U / 1));
			break;
		case 3:
			SoftwareTimerChange(m_nTimerId, (1000U / 3));
			break;
		case 5:
			SoftwareTimerChange(m_nTimerId, (1000U / 5));
			break;
		case 8:
			SoftwareTimerChange(m_nTimerId, (1000U / 8));
			break;
# endif
		case 255:
			SoftwareTimerDelete(m_nTimerId);
#if defined (CONFIG_LEDBLINK_USE_PANELLED)
			hal::panel_led_on(hal::panelled::ACTIVITY);
#else
			GPIO_BOP(LED_BLINK_GPIO_PORT) = LED_BLINK_PIN;
#endif
			break;
		default:
			SoftwareTimerChange(m_nTimerId, (1000U / nFreqHz));
			break;
		}

		DEBUG_EXIT
	}

private:
#if !defined(DISABLE_RTC)
	HwClock m_HwClock;
#endif
	hardware::ledblink::Mode m_Mode { hardware::ledblink::Mode::UNKNOWN };
	bool m_doLock { false };
	TimerHandle_t m_nTimerId { TIMER_ID_NONE };

#if !defined(HAL_HAVE_PORT_BIT_TOGGLE)
	static inline int32_t m_nToggleLed { 1 };
#endif
	static inline Hardware *s_pThis;
};

#endif /* GD32_HARDWARE_H_ */
