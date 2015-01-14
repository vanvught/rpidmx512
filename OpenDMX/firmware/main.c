/**
 * @file main.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <stdio.h>
#include "bcm2835.h"
#include "bcm2835_led.h"
#include "bcm2835_wdog.h"
#include "sys_time.h"
#include "hardware.h"
#include "fiq.h"
#include "bw_ui.h"
#include "ui_functions.h"
#include "dmx_data.h"
#include "dmx_devices.h"

extern void fb_init(void);

struct _poll {
	void (*f)(void);
	const char *msg;
} const poll_table[] = {
	{dmx_devices_run, "DMX Slave" }
};

#ifdef DEBUG
extern void print_devices_counter(void);
#endif

struct _event {
	uint64_t period;
	void (*f)(void);
} const events[] = {
	{1000000, ui_buttons_update},
#ifdef DEBUG
	{1000000, print_devices_counter},
#endif
	{1000000, ui_lcd_refresh}
};

uint64_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 *
 */
inline static void events_init() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += st_read_now;
	}
}

/**
 *
 */
inline static void events_check() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (st_read_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			watchdog_feed();
		}
	}
}

/**
 * @ingroup firmware
 *
 *
 *
 * @param boot_dev
 * @param arm_m_type
 * @param atags
 * @return
 */
int notmain(uint32_t boot_dev, uint32_t arm_m_type, uint32_t atags)
{
	__disable_irq();
	__disable_fiq();

	led_init();
	led_set(1);

	// Read RTC
	sys_time_init();

	// Framebuffer
	fb_init();

	printf("DMX512 Receiver\n");
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);

	// User Interface
	ui_start(0x00);
	ui_reinit();
	ui_text_line_1("DMX512 Receiver", 15);
	ui_text_line_2("Booting........", 15);

	dmx_devices_read_config();
	dmx_devices_init();

	// Led blink 1Hz
	irq_init();
	__enable_irq();

	// PL011 UART
	pl011_dmx512_init();
	fiq_init();
	__enable_fiq();

	watchdog_init();

	events_init();

	while (1) {
		watchdog_feed();

		int i = 0;
		for (i = 0; i < sizeof(poll_table) / sizeof(poll_table[0]); i++) {
			poll_table[i].f();
		}

		events_check();
	}

	return 0;
}
