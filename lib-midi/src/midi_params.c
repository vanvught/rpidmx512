/**
 * @file midi_params.c
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
#include <stddef.h>

#include "read_config_file.h"
#include "sscan.h"

#include "midi.h"
#include "midi_interface.h"

#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "midi.txt";			///< Parameters file name
static const char PARAMS_BAUDRATE[] ALIGNED = "baudrate";			///<
static const char PARAMS_INTERFACE[] ALIGNED = "interface";			///<

static uint32_t midi_baudrate = MIDI_BAUDRATE_DEFAULT;				///<
static _midi_interfaces midi_interface = MIDI_INTERFACE_IN_UART;	///< Default to UART

const _midi_interfaces midi_params_get_interface(void) {
	return midi_interface;
}

const uint32_t midi_params_get_baudrate(void) {
	return midi_baudrate;
}

static void process_line_read(const char *line) {
	uint8_t value8;
	uint32_t value32;

	if (sscan_uint8_t(line, PARAMS_INTERFACE, &value8) == 2) {
		midi_interface = value8;
	}

	if (sscan_uint32_t(line, PARAMS_BAUDRATE, &value32) == 2) {
		if (value32 == 0) {
			midi_baudrate = MIDI_BAUDRATE_DEFAULT;
		} else {
			midi_baudrate = value32;
		}
	}
}

void midi_params_init(void) {
	(void) read_config_file(PARAMS_FILE_NAME, &process_line_read);
}
