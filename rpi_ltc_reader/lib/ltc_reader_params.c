/**
 * @file ltc_reader_params.c
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

#include <stdint.h>
#include <stdbool.h>

#include "ltc_reader_params.h"

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "ltc.txt";
static const char PARAMS_CONSOLE_OUTPUT[] ALIGNED = "console_output";
static const char PARAMS_LCD_OUTPUT[] ALIGNED = "lcd_output";
static const char PARAMS_OLED_OUTPUT[] ALIGNED = "oled_output";
static const char PARAMS_7SEGMENT_OUTPUT[] ALIGNED = "7segment_output";
static const char PARAMS_MIDI_OUTPUT[] ALIGNED = "midi_output";
static const char PARAMS_ARTNET_OUTPUT[] ALIGNED = "artnet_output";
static const char PARAMS_MATRIX_OUTPUT[] ALIGNED = "matrix_output";
static const char PARAMS_SOURCE[] ALIGNED = "source";
static const char PARAMS_MAX7219_INTENSITY[] ALIGNED = "max7219_intensity";

static bool params_console_output = true;
static bool params_lcd_output = true;
static bool params_oled_output = true;
static bool params_7segment_output = false;
static bool params_midi_output = false;
static bool params_artnet_output = false;
static bool params_matrix_output = false;
static uint8_t params_max7219_intensity = 0x04;

static ltc_reader_source_t params_source = LTC_READER_SOURCE_LTC;

const bool ltc_reader_params_is_console_output(void) {
	return params_console_output;
}

const bool ltc_reader_params_is_lcd_output(void) {
	return params_lcd_output;
}

const bool ltc_reader_params_is_oled_output(void) {
	return params_oled_output;
}

const bool ltc_reader_params_is_7segment_output(void) {
	return params_7segment_output;
}

const bool ltc_reader_params_is_midi_output(void) {
	return params_midi_output;
}

const bool ltc_reader_params_is_artnet_output(void) {
	return params_artnet_output;
}

const bool ltc_reader_params_is_matrix_output(void) {
	return params_matrix_output;
}

const ltc_reader_source_t ltc_reader_params_get_source(void) {
	return params_source;
}

const uint8_t ltc_reader_params_get_max7219_intensity(void) {
	return params_max7219_intensity;
}

static void process_line_read(const char *line) {
	uint8_t value8;
	char source[16];
	uint8_t len = sizeof(source);

	if (sscan_uint8_t(line, PARAMS_CONSOLE_OUTPUT, &value8) == 2) {
		if (value8 == 0) {
			params_console_output = false;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_LCD_OUTPUT, &value8) == 2) {
		if (value8 == 0) {
			params_lcd_output = false;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_OLED_OUTPUT, &value8) == 2) {
		if (value8 == 0) {
			params_oled_output = false;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_7SEGMENT_OUTPUT, &value8) == 2) {
		if (value8 == 1) {
			params_7segment_output = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_MIDI_OUTPUT, &value8) == 2) {
		if (value8 == 1) {
			params_midi_output = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_ARTNET_OUTPUT, &value8) == 2) {
		if (value8 == 1) {
			params_artnet_output = true;
		}
		return;
	}

	if (sscan_uint8_t(line, PARAMS_MATRIX_OUTPUT, &value8) == 2) {
		if (value8 == 1) {
			params_matrix_output = true;
		}
		return;
	}

	if (sscan_char_p(line, PARAMS_SOURCE, source, &len) == 2) {
		if (strncasecmp(source, "artnet", len) == 0) {
			params_source = LTC_READER_SOURCE_ARTNET;
		} else if (strncasecmp(source, "midi", len) == 0) {
			params_source = LTC_READER_SOURCE_MIDI;
		}
	}

	if (sscan_uint8_t(line, PARAMS_MAX7219_INTENSITY, &value8) == 2) {
		if (value8 <= 0x0F) {
			params_max7219_intensity = value8;
		}
		return;
	}

}

void ltc_reader_params_init(void) {
	(void) read_config_file(PARAMS_FILE_NAME, &process_line_read);

	if (params_7segment_output && params_matrix_output) {
		params_7segment_output = false;
	}
}
