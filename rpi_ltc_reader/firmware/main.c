/**
 * @file main.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardware.h"
#include "console.h"

#include "lcd.h"
#include "midi_send.h"

#include "ltc_reader.h"
#include "ltc_reader_params.h"

#include "software_version.h"

static struct _ltc_reader_output output = { true, false, false, false };

static void handle_bool(const bool b) {
	if (b) {
		console_save_color();
		console_set_fg_color(CONSOLE_GREEN);
		console_puts("Yes");
		console_restore_color();
	} else {
		console_puts("No");
	}
}

void notmain(void) {
	hardware_init();

	ltc_reader_params_init();

	output.console_output = ltc_reader_params_is_console_output();
	output.lcd_output = ltc_reader_params_is_lcd_output();
	output.midi_output = ltc_reader_params_is_midi_output();
	output.artnet_output = ltc_reader_params_is_artnet_output();

	if(output.midi_output) {
		midi_send_init();
	}

	if (output.lcd_output) {
		output.lcd_output = lcd_detect();
	}

	ltc_reader_init(&output);

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("SMPTE TimeCode LTC Reader");

	console_set_top_row(3);

	console_set_cursor(0, 15);
	console_puts("Console output : "); handle_bool(output.console_output); console_putc('\n');
	console_puts("LCD output     : "); handle_bool(output.lcd_output); console_putc('\n');
	console_puts("MIDI output    : "); handle_bool(output.midi_output); console_putc('\n');
	console_puts("ArtNet output  : "); handle_bool(output.artnet_output);

	for (;;) {
		ltc_reader();
	}
}

