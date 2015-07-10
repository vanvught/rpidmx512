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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bcm2835_wdog.h"
#include "hardware.h"
#include "util.h"
#include "rdm_device_info.h"
#include "dmx.h"
#include "dmx_devices.h"
#include "monitor.h"

// poll table
extern void rdm_data_received(void);
// events table
extern void rdm_identify(void);

struct _poll
{
	void (*f)(void);
}const poll_table[] = {
	{ rdm_data_received },
	{ dmx_devices_run },
};

struct _event
{
	const uint32_t period;
	void (*f)(void);
}const events[] = {
		{  500000, rdm_identify },
		{ 1000000, monitor_update }};

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

int notmain(void) {
	hardware_init();
	dmx_init();
	dmx_devices_read_config();
	dmx_devices_init();
	rdm_device_info_init();

	hardware_print_board_model();
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("RDM Responder, DMX512 data analyzer for 32 channels\n");
	const uint8_t *uid_device = rdm_device_info_get_uuid();
	printf("Device UUID : %.2x%.2x:%.2x%.2x%.2x%.2x\n", uid_device[0], uid_device[1], uid_device[2], uid_device[3], uid_device[4], uid_device[5]);

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
