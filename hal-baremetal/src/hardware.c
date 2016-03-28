/**
 * @file hardware.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <time.h>

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_led.h"
#include "bcm2835_wdog.h"
#include "bcm2837_gpio_virt.h"
#include "mcp7941x.h"

#include "hardware.h"
#include "console.h"
#include "sys_time.h"
#include "ff.h"
#include "smp.h"

static const char FIRMWARE_COPYRIGHT[] __attribute__((aligned(4))) = "Copyright (c) 2012 Broadcom";					///<
static const uint8_t FIRMWARE_COPYRIGHT_LENGTH = (sizeof(FIRMWARE_COPYRIGHT) / sizeof(FIRMWARE_COPYRIGHT[0])) - 1;	///< Length of \ref FIRMWARE_COPYRIGHT

struct _hardware_led {
	void (*init)(void);			///< Pointer to function for LED ACCT init (GPIO FSEL OUTPUT)
	void (*set)(const int);		///< Pointer to function for LED ACCT on/off
}static _hardware_led_f = { led_init, led_set };

#define MAX_NAME_LENGTH 20		///< Length for model name

///< Reference http://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
struct _hardware_revision_code {
	const uint32_t value;
	const char name[MAX_NAME_LENGTH + 1];	///< Including '\0' byte
}const board_version[] __attribute__((aligned(4))) = {
		{ 0x000000, "Model Unknown       " },
		{ 0x000002, "Model B R1 256MB    " },
		{ 0x000003, "Model B R1 256MB    " },
		{ 0x000004, "Model B R2 256MB    " },
		{ 0x000005, "Model B R2 256MB    " },
		{ 0x000006, "Model B R2 256MB    " },
		{ 0x000007, "Model A 256MB       " },
		{ 0x000008, "Model A 256MB       " },
		{ 0x000009, "Model A 256MB       " },
		{ 0x00000d, "Model B R2 512MB    " },
		{ 0x00000e, "Model B R2 512MB    " },
		{ 0x00000f, "Model B R2 512MB    " },
		{ 0x000010, "Model B+ 512MB      " },
		{ 0x000011, "Compute Module 512MB" },
		{ 0x000012, "Model A+ 256MB      " },
		{ 0xa01041, "Pi 2 Model B 1GB    " },
		{ 0xa02082, "Pi 3 Model B 1GB    " },
		{ 0xa21041, "Pi 2 Model B 1GB    " },
		{ 0x900092, "PiZero 512MB        " }
};

static volatile uint64_t hardware_init_startup_micros = 0;	///<

static FATFS fat_fs;		/* File system object */

/**
 * @ingroup hal
 *
 * @return The board uptime in seconds
 */
const uint64_t hardware_uptime_seconds(void) {
	return (((bcm2835_st_read() - hardware_init_startup_micros) / 1E6));
}

/**
 * @ingroup hal
 *
 * @return
 */
const int32_t hardware_get_firmware_revision(void) {
	return bcm2835_vc_get_get_firmware_revision();
}

/**
 * @ingroup hal
 *
 * @return
 */
const char *hardware_get_firmware_copyright(void) {
	return FIRMWARE_COPYRIGHT;
}

/**
 * @ingroup hal
 *
 * @return
 */
const uint8_t hardware_get_firmware_copyright_length(void) {
	return FIRMWARE_COPYRIGHT_LENGTH;
}

/**
 * @ingroup hal
 *
 * @return
 */
const int32_t hardware_get_board_model_id(void) {
	return bcm2835_vc_get_get_board_revision();
}

/**
 * @ingroup hal
 *
 * @return
 */
const char *hardware_get_board_model(void) {
	const uint8_t array_length = sizeof(board_version) / sizeof(board_version[0]);
	const int32_t revision_code = bcm2835_vc_get_get_board_revision();

	uint8_t i;

	if (revision_code <= 0) {
		return board_version[0].name;
	}

	for (i = 1; i < array_length; i++) {
		if ((uint32_t)revision_code == board_version[i].value) {
			return board_version[i].name;
		}
	}

	return board_version[0].name;
}

/**
 * @ingroup hal
 *
 * @return
 */
const uint8_t hardware_get_board_model_length(void) {
	return MAX_NAME_LENGTH;
}

/**
 * @ingroup hal
 *
 */
void hardware_led_init(void) {
	_hardware_led_f.init();
}

/**
 * @ingroup hal
 *
 * @param state
 */
void hardware_led_set(const int state) {
	_hardware_led_f.set(state);
}

/**
 * @ingroup hal
 *
 * @param tm_hw
 */
void hardware_rtc_set(const struct hardware_time *tm_hw) {
	struct rtc_time tm_rtc;
	struct tm tmbuf;

	tm_rtc.tm_hour = (int)tm_hw->hour;
	tm_rtc.tm_min = (int)tm_hw->minute;
	tm_rtc.tm_sec = (int)tm_hw->second;
	tm_rtc.tm_mday = (int)tm_hw->day;
	//tm_rtc.tm_wday = // TODO
	tm_rtc.tm_mon = (int)tm_hw->month;
	tm_rtc.tm_year = (int)tm_hw->year - 2000;	// RTC stores 2 digits only

	if (mcp7941x_start(0x00) != MCP7941X_ERROR) {
		mcp7941x_set_date_time(&tm_rtc);
	}

	tmbuf.tm_hour = tm_rtc.tm_hour;
	tmbuf.tm_min = tm_rtc.tm_min;
	tmbuf.tm_sec = tm_rtc.tm_sec;
	tmbuf.tm_mday = tm_rtc.tm_mday;
	tmbuf.tm_wday = tm_rtc.tm_wday;
	tmbuf.tm_mon = tm_rtc.tm_mon;
	tmbuf.tm_year = tm_rtc.tm_year;
	tmbuf.tm_isdst = 0;

	sys_time_set(&tmbuf);
}

/**
 * @ingroup hal
 *
 */
void hardware_print_board_model(){
	console_puts(hardware_get_board_model());
}

const int32_t hardware_get_core_temperature(void) {
	return bcm2835_vc_get_temperature() / 1000;
}

/**
 * @ingroup hal
 *
 */
void hardware_init(void) {
#if defined ( RPI2 ) || defined ( RPI3 )
#ifndef ARM_ALLOW_MULTI_CORE
	// put all secondary cores to sleep
	uint8_t core_number = 1;
	for (core_number = 1 ; core_number < 4; core_number ++) {
		*(uint32_t *) (SMP_CORE_BASE + (core_number * 0x10)) = (uint32_t) _init_core;
	}
#endif
#endif
	console_init();

	hardware_init_startup_micros = bcm2835_st_read();

	sys_time_init();

	(void) bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_ON_WAIT);

#if (_FFCONF == 82786)	/* R0.09b */
	(void) f_mount((BYTE) 0, &fat_fs);
#elif (_FFCONF == 32020)/* R0.11 */
	(void) f_mount(&fat_fs, (const TCHAR *) "", (BYTE) 1);
#else
#error Not a recognized/tested FatFs version
#endif

	const uint32_t board_revision = bcm2835_vc_get_get_board_revision();

	if (board_revision == 0xa02082) {
#if defined (RPI3)
		_hardware_led_f.init = bcm2837_gpio_virt_init;
		_hardware_led_f.set = bcm2837_gpio_virt_led_set;
#endif
	} else if (board_revision > 0x00000f) {
		_hardware_led_f.init = led_rpiplus_init;
		_hardware_led_f.set = led_rpiplus_set;
	}

	hardware_led_init();
	hardware_led_set(1);
}

/**
 * @ingroup hal
 *
 */
void hardware_reboot(void) {
	hardware_led_set(1);
	watchdog_init();
	for (;;)
		;
}
