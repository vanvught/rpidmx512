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

#include "bcm2835.h"
#include "bcm2835_led.h"
#include "bcm2835_wdog.h"
#include "sys_time.h"
#include "ft245rl.h" // TODO
#include "dmx_data.h"

extern void fb_init(void);

// poll table
extern void receive_data_from_host(void);
// events table
extern void received_dmx_packet(void);
extern void received_dmx_change_of_state_packet(void);
// external monitor
extern void monitor_update(void);

struct _poll
{
	void (*f)(void);
} const poll_table[] = {
		{ receive_data_from_host } };

struct _event
{
	uint32_t period;
	void (*f)(void);
}const events[] = {
		{ 800000, received_dmx_packet },
		{ 800000, received_dmx_change_of_state_packet },
		{1000000, monitor_update }};

uint64_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 *
 */
static void events_init() {
	int i;
	uint64_t st_read_now = bcm2835_st_read();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		events_elapsed_time[i] += st_read_now;
	}
}

/**
 *
 */
static inline void events_check() {
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

int notmain(void)
{
	FT245RL_init();  // TODO

	dmx_init();
	
	led_init();
	led_set(1);

	sys_time_init();	// Read RTC

	fb_init();			// Framebuffer

	printf("Compiled on %s at %s\n", __DATE__, __TIME__);

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
