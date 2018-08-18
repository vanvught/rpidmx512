/**
 * @file hardware.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <time.h>

#include "c/sys_time.h"

#include "arm/synchronize.h"
#include "arm/smp.h"

#include "bcm2835_st.h"
#include "bcm2835_vc.h"
#include "bcm2835_led.h"
#include "bcm2835_rng.h"
#include "bcm2837_gpio_virt.h"

#include "console.h"
#include "ff.h"


struct _hardware_led {
	void (*init)(void);		///< Pointer to function for LED ACCT init (GPIO FSEL OUTPUT)
	void (*set)(const int);	///< Pointer to function for LED ACCT on/off
}static _hardware_led_f = { led_init, led_set };

static volatile uint64_t hardware_init_startup_micros = 0;	///<

#if (_FFCONF == 68300)
 static FATFS fat_fs;		/* File system object */
#endif

uint64_t hardware_uptime_seconds(void) {
	return (uint64_t) (bcm2835_st_read() - hardware_init_startup_micros) / (uint64_t) 1000000;
}

void hardware_led_init(void) {
	_hardware_led_f.init();
}

void hardware_led_set(const int state) {
	_hardware_led_f.set(state);
}

int32_t hardware_get_core_temperature(void) {
	return bcm2835_vc_get_temperature() / 1000;
}

void hardware_init(void) {
	int32_t board_revision;

#if defined ( RPI2 ) || defined ( RPI3 )
#ifndef ARM_ALLOW_MULTI_CORE
	// put all secondary cores to sleep
	uint8_t core_number = 1;
	for (core_number = 1 ; core_number < 4; core_number ++) {
		*(uint32_t *) (SMP_CORE_BASE + (core_number * 0x10)) = (uint32_t) _init_core;
	}
#endif
#endif

	(void) console_init();
	hardware_init_startup_micros = bcm2835_st_read();
	sys_time_init();
	bcm2835_rng_init();

	(void) bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_ON_WAIT);

#if (_FFCONF == 68300)/*R0.12c *//* 32020 R0.11 */
	FRESULT result = f_mount(&fat_fs, (const TCHAR *) "", (BYTE) 1);
	if (result != FR_OK) {
		char buffer[32];
		snprintf(buffer, 31, "f_mount failed! %d\n", (int) result);
		console_error(buffer);
	}
#else
#error Not a recognized/tested FatFs version
#endif

	board_revision = bcm2835_vc_get_get_board_revision();

	if ((board_revision == 0xa02082) || (board_revision == 0xa22082) || (board_revision == 0xa020d3)) { //TODO board_revision check
		_hardware_led_f.init = bcm2837_gpio_virt_init;
		_hardware_led_f.set = bcm2837_gpio_virt_led_set;
	} else if (board_revision > 0x00000f) {
		_hardware_led_f.init = led_rpiplus_init;
		_hardware_led_f.set = led_rpiplus_set;
	}

	hardware_led_init();
	hardware_led_set(1);
}
