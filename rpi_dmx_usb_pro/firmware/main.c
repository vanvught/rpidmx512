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

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "bcm2835_wdog.h"
#include "usb.h"
#include "hardware.h"
#include "dmx.h"
#include "widget_params.h"
#include "rdm_device_info.h"
#include "widget.h"
#include "monitor.h"

struct _poll
{
	void (*f)(void);
} const poll_table[] = {
		{ widget_receive_data_from_host },
		{ widget_received_dmx_packet },
		{ widget_received_dmx_change_of_state_packet },
		{ widget_received_rdm_packet },
		{ widget_rdm_timeout },
		{ widget_sniffer_rdm },
		{ widget_sniffer_dmx }
		};

struct _event
{
	const uint32_t period;
	void (*f)(void);
}const events[] = {
		{1000000, monitor_update }
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
		if (micros_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			watchdog_feed();
		}
	}
}

/**
 * @ingroup main
 *
 * @return
 */
int notmain(void)
{
	hardware_init();
	usb_init();
	dmx_init();
	widget_params_init();
	rdm_device_info_init();

	hardware_print_board_model();
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("RDM Controller with USB [Compatible with Enttec USB Pro protocol]\n");
	const uint8_t *device_sn = rdm_device_info_get_sn();
	printf("Widget mode %d, S/N : %.2X%.2X%.2X%.2X\n", widget_get_mode(), device_sn[3], device_sn[2], device_sn[1], device_sn[0]);

	watchdog_init();

	events_init();

	for (;;)
	{
		watchdog_feed();
		int i = 0;
		for (i = 0; i < sizeof(poll_table) / sizeof(poll_table[0]); i++)
		{
			poll_table[i].f();
		}

		events_check();
	}

	return 0;
}
