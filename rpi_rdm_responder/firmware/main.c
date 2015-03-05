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

#include "bcm2835.h"
#include "bcm2835_led.h"
#include "bcm2835_wdog.h"
#include "hardware.h"

#include "dmx.h"
#include "device_info.h"

typedef enum {
	FALSE = 0,
	TRUE = 1
} _boolean;


// poll table
extern void rdm_handle_data(void);
// events table
extern void monitor_update(void);

void task_led(void) {
	static unsigned char led_counter = 0;
	led_set(led_counter++ & 0x01);
}

struct _poll
{
	void (*f)(void);
}const poll_table[] = {
		{ rdm_handle_data } };

struct _event
{
	uint64_t period;
	void (*f)(void);
}const events[] = {
		{ 1000000, monitor_update },
		{  500000, task_led } };

uint64_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

static void events_init() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += st_read_now;
	}
}

static inline void events_check() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (st_read_now > events_elapsed_time[i] + events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			//watchdog_feed();
		}
	}
}

int notmain(void) {
	hardware_init();
	dmx_init();
	device_info_init();

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("RDM Responder, DMX512 data analyzer for 32 channels\n");

	dmx_port_direction_set(DMX_PORT_DIRECTION_INP, TRUE);

	//watchdog_init();

	events_init();

	for (;;)
	{
		//watchdog_feed();
		int i = 0;
		for (i = 0; i < sizeof(poll_table) / sizeof(poll_table[0]); i++)
		{
			poll_table[i].f();
		}

		events_check();
	}

	return 0;
}
