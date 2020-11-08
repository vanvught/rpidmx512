/**
 * @file hardware_init.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "h3_board.h"
#include "h3_gpio.h"
#include "h3_sid.h"
#include "h3_thermal.h"
#include "h3.h"
#include "h3_cpu.h"
#include "h3_watchdog.h"
#include "h3_i2c.h"
#include "h3_spi.h"

#include "arm/gic.h"
#include "arm/synchronize.h"

#include "c/sys_time.h"

#include "device/emac.h"

#include "console.h"
#include "../ff12c/ff.h"

#define WIFI_EN_PIO		7	// PL7
#define POWER_LED_PIO	10	// PL10
#define EXTERNAL_LED 	GPIO_EXT_16

#if (_FFCONF == 68300)		// R0.12c
 static FATFS fat_fs;
#else
# error Not a recognized/tested FatFs version
#endif

#if defined (ORANGE_PI_ONE) && defined(ENABLE_PWR_BUTTON)
 static bool s_is_pwr_button_pressed = false;
#endif

static uint32_t s_hardware_init_startup_seconds = 0;

extern void sys_time_init(void);
extern void h3_timer_init(void);
extern void h3_hs_timer_init(void);
extern void h3_usb_end(void);

uint32_t hardware_uptime_seconds(void) {
	return (H3_TIMER->AVS_CNT0 / 1000) - s_hardware_init_startup_seconds;
}

int32_t hardware_get_mac_address(/*@out@*/uint8_t *mac_address) {
	const uint32_t mac_lo = H3_EMAC->ADDR[0].LOW;
	const uint32_t mac_hi = H3_EMAC->ADDR[0].HIGH;

#ifndef NDEBUG
	printf ("H3_EMAC->ADDR[0].LOW=%08x, H3_EMAC->ADDR[0].HIGH=%08x\n", mac_lo, mac_hi);
#endif

	mac_address[0] = (mac_lo >> 0) & 0xff;
	mac_address[1] = (mac_lo >> 8) & 0xff;
	mac_address[2] = (mac_lo >> 16) & 0xff;
	mac_address[3] = (mac_lo >> 24) & 0xff;
	mac_address[4] = (mac_hi >> 0) & 0xff;
	mac_address[5] = (mac_hi >> 8) & 0xff;

#ifndef NDEBUG
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
#endif

	return 0;
}

void hardware_led_init(void) {
	h3_gpio_fsel(H3_BOARD_STATUS_LED, GPIO_FSEL_OUTPUT);
#if !defined(DO_NOT_USE_EXTERNAL_LED)
	h3_gpio_fsel(EXTERNAL_LED, GPIO_FSEL_OUTPUT);
#endif
}

void hardware_led_set(int state) {
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
	#define MASK_LED 		(((uint32_t)1 << H3_BOARD_STATUS_LED) | ((uint32_t)1 << EXTERNAL_LED))
#else
	#define MASK_LED 		((uint32_t)1 << H3_BOARD_STATUS_LED)
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

void __attribute__((cold)) hardware_init(void) {
	h3_watchdog_disable();
	h3_usb_end();
	h3_timer_init();
	h3_hs_timer_init();
	sys_time_init();
	console_init();
	gic_init();
	h3_thermal_init();
	emac_init();
	h3_spi_begin();
	h3_i2c_begin();

	s_hardware_init_startup_seconds = H3_TIMER->AVS_CNT0 / 1000;

#ifndef ARM_ALLOW_MULTI_CORE
	uint8_t cpu_number;
	for (cpu_number = 1 ; cpu_number < H3_CPU_COUNT; cpu_number ++) {
		h3_cpu_off(cpu_number);
	}
#endif

	const FRESULT result = f_mount(&fat_fs, (const TCHAR *) "", (BYTE) (h3_get_boot_device() == H3_BOOT_DEVICE_MMC0) ? 1 : 0);
	if (result != FR_OK) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer) - 1, "f_mount failed! %d\n", (int) result);
		console_error(buffer);
		assert(0);
	}

#define PRCM_APB0_GATE_PIO (0x1 << 0)
	H3_PRCM->APB0_GATE |= PRCM_APB0_GATE_PIO;
#define PRCM_APB0_RESET_PIO (0x1 << 0)
	H3_PRCM->APB0_RESET |= PRCM_APB0_RESET_PIO;
	uint32_t value = H3_PIO_PORTL->CFG1;
	value &= (uint32_t) ~(GPIO_SELECT_MASK << PL10_SELECT_CFG1_SHIFT);
	value |= (GPIO_FSEL_OUTPUT << PL10_SELECT_CFG1_SHIFT);
	H3_PIO_PORTL->CFG1 = value;
	// Power led on, disable WiFi
	value = H3_PIO_PORTL->DAT;
	value &= ~(1U << WIFI_EN_PIO);
	value |= (1U << POWER_LED_PIO);
	H3_PIO_PORTL->DAT = value;

#if defined (ORANGE_PI_ONE) && defined(ENABLE_PWR_BUTTON)
	// PWR-KEY
	value = H3_PIO_PORTL->CFG0;
	value &= (uint32_t)~(GPIO_SELECT_MASK << PL3_SELECT_CFG0_SHIFT);
	value |= (GPIO_FSEL_INPUT << PL3_SELECT_CFG0_SHIFT);
	H3_PIO_PORTL->CFG0 = value;
	s_is_pwr_button_pressed = (H3_PIO_PORTL->DAT & (1 << 3)) == 0;
#endif

	hardware_led_init();
	hardware_led_set(1);

	h3_cpu_set_clock(0); // default
}
