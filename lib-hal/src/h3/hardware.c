/**
 * @file hardware.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_gpio.h"
#include "h3_sid.h"
#include "h3_thermal.h"
#include "h3.h"
#include "h3_cpu.h"
#include "h3_watchdog.h"

#include "arm/gic.h"

#include "c/sys_time.h"

#include "device/emac.h"

#include "console.h"
#include "ff.h"

#define POWER_LED_PIO	10	// PL10

#if (_FFCONF == 68300)		// R0.12c
 static FATFS fat_fs;
#endif

static bool s_is_pwr_button_pressed = false;
static volatile uint64_t s_hardware_init_startup_seconds = 0;

bool hardware_is_pwr_button_pressed(void) {
	return s_is_pwr_button_pressed;
}

uint64_t hardware_uptime_seconds(void) {
	const uint64_t now = h3_read_cnt64() / (uint64_t) (24 * 1000000);
	return (now - s_hardware_init_startup_seconds);
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
}

void hardware_led_set(int state) {
	if (state == 0) {
		h3_gpio_clr(H3_BOARD_STATUS_LED);
	} else {
		h3_gpio_set(H3_BOARD_STATUS_LED);
	}
}

void hardware_init(void) {
	h3_watchdog_disable();
	sys_time_init();
	h3_timer_init();
	h3_hs_timer_start();
	console_init();
	gic_init();
	h3_thermal_init();
	emac_init();

	s_hardware_init_startup_seconds = h3_read_cnt64() / (uint64_t) (24 * 1000000);

#ifndef ARM_ALLOW_MULTI_CORE
	// put all secondary cores to sleep
	uint8_t cpu_number = 1;
	for (cpu_number = 1 ; cpu_number < H3_CPU_COUNT; cpu_number ++) {
		h3_cpu_off(cpu_number);
	}
#endif

#if (_FFCONF == 68300)/*R0.12c */
	FRESULT result = f_mount(&fat_fs, (const TCHAR *) "", (BYTE) 1);
	if (result != FR_OK) {
		char buffer[32];
		snprintf(buffer, 31, "f_mount failed! %d\n", (int) result);
		console_error(buffer);
		assert(0);
	}
#else
#error Not a recognized/tested FatFs version
#endif

	// Power led
	#define PRCM_APB0_GATE_PIO (0x1 << 0)
	H3_PRCM->APB0_GATE |= PRCM_APB0_GATE_PIO;
	#define PRCM_APB0_RESET_PIO (0x1 << 0)
	H3_PRCM->APB0_RESET |= PRCM_APB0_RESET_PIO;
	uint32_t value = H3_PIO_PORTL->CFG1;
	value &= ~(GPIO_SELECT_MASK << PL10_SELECT_CFG1_SHIFT);
	value |= (GPIO_FSEL_OUTPUT << PL10_SELECT_CFG1_SHIFT);
	H3_PIO_PORTL->CFG1 = value;
	// Set on
	H3_PIO_PORTL->DAT |= 1 << POWER_LED_PIO;

#if defined (ORANGE_PI_ONE)
	// PWR-KEY
	value = H3_PIO_PORTL->CFG0;
	value &= ~(GPIO_SELECT_MASK << PL3_SELECT_CFG0_SHIFT);
	value |= (GPIO_FSEL_INPUT << PL3_SELECT_CFG0_SHIFT);
	H3_PIO_PORTL->CFG0 = value;
	s_is_pwr_button_pressed = (H3_PIO_PORTL->DAT & (1 << 3)) == 0;
#endif

	hardware_led_init();
	hardware_led_set(1);

	h3_cpu_set_clock(0); // default
}
