/**
 * @file hardware_init.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>
#include <time.h>
#include <sys/time.h>

#include "h3.h"
#include "h3_ccu.h"
#include "h3_cpu.h"
#include "h3_watchdog.h"
#include "h3_i2c.h"
#include "h3_spi.h"
#include "h3_board.h"
#include "h3_gpio.h"
#include "h3_sid.h"
#include "h3_thermal.h"

#include "arm/gic.h"
#include "arm/synchronize.h"

#include "console.h"

#include "../ff14b/source/ff.h"

#if defined (DEBUG_I2C)
# include "i2cdetect.h"
#endif

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "logic_analyzer.h"

#include "debug.h"

namespace hal {
extern bool g_bWatchdog;
}  // namespace hal

#define WIFI_EN_PIO		7	// PL7
#define POWER_LED_PIO	10	// PL10
#define EXTERNAL_LED 	GPIO_EXT_16

//#if (_FFCONF == 68300)		// R0.12c
#if (FF_DEFINED	== 86631)		// R0.14b
 static FATFS fat_fs;
#else
# error Not a recognized/tested FatFs version
#endif

static uint32_t s_hardware_init_startup_seconds = 0;

void emac_init();
void h3_timer_avs_init();
void h3_hs_timer_init();
void h3_usb_end();

uint32_t h3_uptime() {
	return (H3_TIMER->AVS_CNT0 / 1000) - s_hardware_init_startup_seconds;
}

void h3_status_led_set(int state) {
#if defined(ORANGE_PI_ONE)
	if (state == 0) {
		h3_gpio_clr(H3_BOARD_STATUS_LED);
#if !defined(DO_NOT_USE_EXTERNAL_LED)
		h3_gpio_clr(EXTERNAL_LED);
#endif
	} else {
		h3_gpio_set(H3_BOARD_STATUS_LED);
#if !defined(DO_NOT_USE_EXTERNAL_LED)
		h3_gpio_set(EXTERNAL_LED);
#endif
	}
#else

#if !defined(DO_NOT_USE_EXTERNAL_LED)
	#define MASK_LED 		static_cast<uint32_t>((1U << H3_BOARD_STATUS_LED) | (1U << EXTERNAL_LED))
#else
	#define MASK_LED 		static_cast<uint32_t>(1U << H3_BOARD_STATUS_LED)
#endif

	uint32_t dat = H3_PIO_PORTA->DAT;

	if (state == 0) {
		dat &= ~(MASK_LED);
	} else {
		dat |= (MASK_LED);
	}

	H3_PIO_PORTA->DAT = dat;
#endif
}

void __attribute__((cold)) hal_init() {
	h3_gpio_fsel(EXT_SPI_MOSI, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_MOSI, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CLK, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CLK, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CS, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CS, GPIO_PULL_DOWN);

	h3_watchdog_disable();
	h3_usb_end();
	h3_timer_avs_init();
	h3_hs_timer_init();
	console_init();
	gic_init();
	h3_thermal_init();
	emac_init();
	h3_i2c_begin();

	console_puts("Starting ...\n");

	s_hardware_init_startup_seconds = H3_TIMER->AVS_CNT0 / 1000U;

	struct tm tmbuf;

	tmbuf.tm_hour = 0;
	tmbuf.tm_min = 0;
	tmbuf.tm_sec = 0;
	tmbuf.tm_mday = _TIME_STAMP_DAY_;			// The day of the month, in the range 1 to 31.
	tmbuf.tm_mon = _TIME_STAMP_MONTH_ - 1;		// The number of months since January, in the range 0 to 11.
	tmbuf.tm_year = _TIME_STAMP_YEAR_ - 1900;	// The number of years since 1900.
	tmbuf.tm_isdst = 0; 						// 0 (DST not in effect, just take RTC time)

	const time_t seconds = mktime(&tmbuf);
	const struct timeval tv = { seconds, 0 };

	settimeofday(&tv, nullptr);

	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", 1900 + tmbuf.tm_year, tmbuf.tm_mon, tmbuf.tm_mday, tmbuf.tm_hour, tmbuf.tm_min, tmbuf.tm_sec);

#ifndef ARM_ALLOW_MULTI_CORE
	for (uint32_t cpu_number = 1 ; cpu_number < H3_CPU_COUNT; cpu_number ++) {
		h3_cpu_off(static_cast<h3_cpu_t>(cpu_number));
	}
#endif

	const FRESULT result = f_mount(&fat_fs, reinterpret_cast<const TCHAR *>(""), static_cast<BYTE>(h3_get_boot_device() == H3_BOOT_DEVICE_MMC0) ? 1 : 0);
	if (result != FR_OK) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer) - 1, "f_mount failed! %d\n", static_cast<int>(result));
		console_error(buffer);
		assert(0);
	}

#define PRCM_APB0_GATE_PIO (0x1 << 0)
	H3_PRCM->APB0_GATE |= PRCM_APB0_GATE_PIO;
#define PRCM_APB0_RESET_PIO (0x1 << 0)
	H3_PRCM->APB0_RESET |= PRCM_APB0_RESET_PIO;
	uint32_t value = H3_PIO_PORTL->CFG1;
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PL10_SELECT_CFG1_SHIFT));
	value |= (GPIO_FSEL_OUTPUT << PL10_SELECT_CFG1_SHIFT);
	H3_PIO_PORTL->CFG1 = value;
	// Power led on, disable WiFi
	value = H3_PIO_PORTL->DAT;
	value &= ~(1U << WIFI_EN_PIO);
	value |= (1U << POWER_LED_PIO);
	H3_PIO_PORTL->DAT = value;

	///< Enable DMA support
	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

	h3_gpio_fsel(H3_BOARD_STATUS_LED, GPIO_FSEL_OUTPUT);
#if !defined(DO_NOT_USE_EXTERNAL_LED)
	h3_gpio_fsel(EXTERNAL_LED, GPIO_FSEL_OUTPUT);
#endif

	h3_status_led_set(0);

	h3_cpu_set_clock(0); // default

	hal::g_bWatchdog = false;

#if defined (DEBUG_I2C)
	i2c_detect();
#endif

#if !defined(DISABLE_RTC)
	HwClock::Get()->RtcProbe();
	HwClock::Get()->Print();
	HwClock::Get()->HcToSys();
#endif

	h3_status_led_set(1);

	logic_analyzer::init();
}
