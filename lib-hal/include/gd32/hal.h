/**
 * @file hal.h
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

#ifndef GD32_HAL_H_
#define GD32_HAL_H_

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "gd32_adc.h"

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

#if defined (USE_FREE_RTOS)
# include "FreeRTOS.h"
# include "task.h"
#endif

#include "softwaretimers.h"

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "panel_led.h"

uint32_t gd32_micros();
extern struct HwTimersSeconds g_Seconds;

#if defined (CONFIG_HAL_USE_SYSTICK)
extern volatile uint32_t gv_nSysTickMillis;
#endif

namespace hal {
extern bool g_bWatchdog;

static constexpr uint32_t BOARD_ID = 0;
static constexpr uint32_t RELEASE_ID = 0;
static constexpr const char WEBSITE[] = "https://gd32-dmx.org";

inline const char *board_name(uint8_t& nLength) {
	nLength = sizeof(GD32_BOARD_NAME) - 1U;
	return GD32_BOARD_NAME;
}

inline const char *soc_name(uint8_t& nLength) {
	nLength = 4;
	return "GD32";
}

inline const char *cpu_name(uint8_t& nLength) {
	nLength = sizeof(GD32_MCU_NAME) - 1U;
	return GD32_MCU_NAME;
}

inline const char *sys_name(uint8_t& nLength) {
	nLength = 8;
	return "Embedded";
}

inline uint32_t millis() {
#if defined (CONFIG_HAL_USE_SYSTICK)
	return gv_nSysTickMillis;
#elif defined (USE_FREE_RTOS)
	return xTaskGetTickCount();
#else
	uint32_t timer6_get_elapsed_milliseconds();
	return timer6_get_elapsed_milliseconds();
#endif
}

inline uint32_t micros() {
	return gd32_micros();
}

inline uint32_t uptime() {
	return g_Seconds.nUptime;
}

inline bool set_rtc([[maybe_unused]] const struct tm *pTime) {
#if !defined(DISABLE_RTC)
	assert(HwClock::Get() != nullptr);
	HwClock::Get()->Set(pTime);
	return true;
#else
	return false;
#endif
}

static constexpr float CORE_TEMPERATURE_MIN = -40.0;
static constexpr float CORE_TEMPERATURE_MAX = +85.0;

inline float core_temperature_current() {
	return gd32_adc_gettemp();
}

inline void watchdog_init() {
	g_bWatchdog = (SUCCESS == fwdgt_config(0xFFFF, FWDGT_PSC_DIV16));

	if (g_bWatchdog) {
		fwdgt_enable();
	}
}

inline void watchdog_feed() {
	fwdgt_counter_reload();
}

inline void watchdog_stop() {
	g_bWatchdog = false;
	fwdgt_config(0xFFFF, FWDGT_PSC_DIV64);
}

inline bool watchdog() {
	return g_bWatchdog;
}

bool reboot();

inline void run() {
#if defined (ENABLE_USB_HOST) && defined (CONFIG_USB_HOST_MSC)
# if defined (GD32H7XX) || defined (GD32F4XX)
	usbh_core_task(&usb_host_msc);
# else
	usbh_core_task(&usb_host);
# endif
#endif
#if !defined(USE_FREE_RTOS)
	SoftwareTimerRun();
#endif
	hal::panel_led_run();
#if defined (DEBUG_STACK)
	stack_debug_run();
#endif
#if defined (DEBUG_EMAC)
	emac_debug_run();
#endif
}
}  // namespace hal

#endif /* GD32_HAL_H_ */
