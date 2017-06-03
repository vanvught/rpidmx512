/**
 * @file display_oled.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>

#include "software_version.h"

#include "ltc_reader_params.h"

#include "oled.h"

static oled_info_t oled_info = { OLED_128x64_I2C_DEFAULT };

/**
 *
 */
const bool display_oled_init(void) {
	uint8_t line = 4;
	ltc_reader_source_t source;

	if (!oled_start(&oled_info)) {
		return false;
	}

	source = ltc_reader_params_get_source();

	oled_set_cursor(&oled_info, 3, 0);
	switch (source) {
		case LTC_READER_SOURCE_ARTNET:
			(void) oled_puts(&oled_info, "TimeCode ArtNet");
			break;
		case LTC_READER_SOURCE_MIDI:
			(void) oled_puts(&oled_info, "TimeCode MIDI");
			break;
		default:
			(void) oled_puts(&oled_info, "SMPTE TimeCode LTC");
			break;
	}

	if (source != LTC_READER_SOURCE_MIDI) {
		oled_set_cursor(&oled_info, line++, 0);
		(void) oled_printf(&oled_info, "MIDI output : %s", ltc_reader_params_is_midi_output() ? "Yes" : "No");
	}

	if (source != LTC_READER_SOURCE_ARTNET) {
		oled_set_cursor(&oled_info, line, 0);
		(void) oled_printf(&oled_info, "ArtNet output : %s", ltc_reader_params_is_artnet_output() ? "Yes" : "No");
	}

	oled_set_cursor(&oled_info, 7, 0);
	(void) oled_printf(&oled_info, "[V%s]", SOFTWARE_VERSION);

	return true;
}

/**
 *
 * @param s
 * @param n
 */
void display_oled_line_1(const char *s, int n) {
	oled_set_cursor(&oled_info, 0, 0);
	oled_write(&oled_info, s, n);
}

/**
 *
 * @param s
 */
void display_oled_line_2(const char *s) {
	oled_set_cursor(&oled_info, 1, 0);
	(void) oled_puts(&oled_info, s);
}
