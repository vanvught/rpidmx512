/**
 * @file main.c
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "c/hardware.h"
#include "c/led.h"

#include "console.h"
#include "oled.h"

#include "dmx.h"

#include "midi.h"
#include "midi_params.h"

#include "bridge.h"
#include "bridge_params.h"

#include "software_version.h"

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

extern void bridge_params_init(void);
extern void dmx_init(void);

// Poll table
extern void midi_poll(void);

struct _poll {
	void (*f)(void);
}static const poll_table[] ALIGNED = {
		{ midi_poll },
		{ bridge },
		{ led_blink } };

extern void monitor_update(void);

struct _event {
	const uint32_t period;
	void (*f)(void);
}static const events[] ALIGNED = {
		{ 1000000, monitor_update } };

static uint32_t events_elapsed_time[sizeof(events) / sizeof(events[0])];

/**
 * @ingroup main
 *
 */
static void events_init() {
	size_t i;
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
	size_t i;
	const uint32_t micros_now = hardware_micros();
	for (i = 0; i < (sizeof(events) / sizeof(events[0])); i++) {
		if (micros_now - events_elapsed_time[i] > events[i].period) {
			events[i].f();
			events_elapsed_time[i] += events[i].period;
			hardware_watchdog_feed();
		}
	}
}

void notmain(void) {
	oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };
	bool oled_connected = false;
	uint8_t midi_channel = 0;
	int i;

	oled_connected = oled_start(&oled_info);

	bridge_params_init();
	dmx_init();
	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	midi_params_init();
	midi_set_interface(MIDI_INTERFACE_IN_SPI);
	midi_set_baudrate(midi_params_get_baudrate());
	midi_active_set_sense(true);
	midi_init(MIDI_DIRECTION_INPUT);

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("MIDI In : Baudrate : %d, Interface : %s, mode : %d\n", (int) midi_get_baudrate(), midi_get_interface_description(), (int) bridge_params_get_bridge_mode());
	printf("DMX Out : BreakTime %d(%d) MaBTime %d(%d) RefreshRate %d(%d)",
			(int) bridge_params_get_break_time(), (int) dmx_get_output_break_time(),
			(int) bridge_params_get_mab_time(), (int) dmx_get_output_mab_time(),
			(int) bridge_params_get_refresh_rate(), (int) (1000000 / dmx_get_output_period()));

	if (oled_connected) {
		midi_channel = bridge_params_get_midi_channel();

		oled_set_cursor(&oled_info, 0, 0);
		(void) oled_printf(&oled_info, "[V%s] MIDI->DMX", SOFTWARE_VERSION);
		oled_set_cursor(&oled_info, 2, 0);
		(void) oled_printf(&oled_info, "MIDI In: %d", (int) midi_get_baudrate());
		oled_set_cursor(&oled_info, 3, 0);
		(void) oled_printf(&oled_info, "Mode: %d", (int) bridge_params_get_bridge_mode());
		oled_set_cursor(&oled_info, 4, 0);
		(void) oled_printf(&oled_info, "Chl: %d %s", midi_channel, midi_channel == 0 ? "<OMNI>" : "");
		oled_set_cursor(&oled_info, 6, 0);
		(void) oled_printf(&oled_info, "DMX Address: %d", (int) bridge_params_get_dmx_start_address());
	}

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
