/**
 * @file main.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <midi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"
#include "console.h"
#include "led.h"
#include "dmx.h"
#include "bridge.h"
#include "bridge_params.h"

extern void bridge_params_init(void);
extern void midi_init(void);
extern void dmx_init(void);

// Poll table
extern void midi_poll(void);

struct _poll {
	void (*f)(void);
}const poll_table[] = {
		{ midi_poll },
		{ bridge },
		{ led_blink } };

extern void monitor_update(void);

struct _event {
	const uint32_t period;
	void (*f)(void);
}const events[] = {
		{ 1000000, monitor_update } };

uint32_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 * @ingroup main
 *
 */
static void events_init() {
	int i;
	const uint32_t mircos_now = hardware_micros();
	for (i = 0; i < (int)(sizeof(events) / sizeof(events[0])); i++) {
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
	for (i = 0; i < (int)(sizeof(events) / sizeof(events[0])); i++) {
		if (micros_now - events_elapsed_time[i] > events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			hardware_watchdog_feed();
		}
	}
}

void notmain(void) {
	int i;

	hardware_init();
	bridge_params_init();
	dmx_init();
	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);
	midi_init();

	printf("%s Compiled on %s at %s\n", hardware_get_board_model(), __DATE__, __TIME__);
	printf("MIDI->DMX Bridge, baudrate : %d, interface : %s, mode : %d\n",
			(int) midi_get_baudrate(), midi_get_interface_description(), bridge_params_get_bridge_mode());
	printf("DMX Out : BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)",
			(int)bridge_params_get_break_time(), (int) dmx_get_output_break_time(),
			(int)bridge_params_get_mab_time(), (int) dmx_get_output_mab_time(),
			(int)bridge_params_get_refresh_rate(), (int) (1E6 / dmx_get_output_period()));

	bridge_init();

	hardware_watchdog_init();

	events_init();

	for (;;) {
		hardware_watchdog_feed();
		for (i = 0; i < (int)(sizeof(poll_table) / sizeof(poll_table[0])); i++) {
			poll_table[i].f();
		}

		events_check();
	}
}
