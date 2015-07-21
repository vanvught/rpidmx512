/**
 * @file main.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <dmx_devices.h>
#include <stdio.h>
#include <stdint.h>

#include "sys_time.h"
#include "hardware.h"
#include "bw_ui.h"
#include "irq_led.h"
#include "ui_functions.h"

#include "dmx.h"

extern void monitor_update(void);

struct _poll {
	void (*f)(void);
	const char *msg;
} const poll_table[] = {
	{dmx_devices_run, "DMX Slave" }
};

struct _event {
	const uint32_t period;
	void (*f)(void);
} const events[] = {
	{1000000, ui_buttons_update},
	{1000000, ui_lcd_refresh},
	{1000000, monitor_update}
};

uint32_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 * @ingroup main
 *
 */
static void events_init() {
	int i;
	const uint32_t mircos_now = hardware_micros();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += mircos_now;
	}
}

/**
 * @ingroup main
 *
 */
inline static void events_check() {
	int i;
	const uint32_t micros_now = hardware_micros();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (micros_now - events_elapsed_time[i] >  events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			hardware_watchdog_feed();
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
int notmain(uint32_t boot_dev, uint32_t arm_m_type, uint32_t atags) {
	hardware_init();
	dmx_init();
	dmx_devices_read_config();

	hardware_print_board_model();
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("Devices connected : %d\n", dmx_devices_get_devices_connected());

	ui_start(0x00);			// User Interface
	ui_reinit();
	ui_text_line_1("DMX512 Receiver", 15);
	ui_text_line_2("Booting........", 15);

	dmx_devices_init();

	hardware_watchdog_init();

	events_init();

	for (;;) {
		hardware_watchdog_feed();
		int i = 0;
		for (i = 0; i < sizeof(poll_table) / sizeof(poll_table[0]); i++) {
			poll_table[i].f();
		}

		events_check();
	}

	return 0;
}
