/**
 * @file sniffer_params.c
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

#include <stdint.h>
#include <stddef.h>

#include "midi.h"
#include "sscan.h"
#include "ff.h"

static const TCHAR FILE_NAME_PARAMS[] = "params.txt";							///< Parameters file name

static const char PARAMS_BAUDRATE[] = "baudrate";								///<
static const char PARAMS_INTERFACE[] = "interface";								///<

static uint32_t midi_baudrate = MIDI_BAUDRATE_DEFAULT;
#if defined (MIDI_DMX_BRIDGE)
static uint8_t midi_interface = 0;	///< SPI
#else
static uint8_t midi_interface = 1;	///< UART
#endif

/**
 *
 * @return
 */
uint8_t sniffer_params_get_interface(void) {
	return midi_interface;
}

/**
 *
 * @return
 */
uint32_t sniffer_params_get_baudrate(void) {
	return midi_baudrate;
}

/**
 *
 * @param line
 */
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


static void read_config_file(void) {
	FRESULT rc = FR_DISK_ERR;
	FIL file_object;

	rc = f_open(&file_object, FILE_NAME_PARAMS, (BYTE) FA_READ);

	if (rc == FR_OK) {
		TCHAR buffer[128];
		for (;;) {
			if (f_gets(buffer, (int) sizeof(buffer), &file_object) == NULL)
				break; // Error or end of file
			(void) process_line_read((const char *) buffer);
		}
		(void) f_close(&file_object);
	} else {
		// nothing to do here
	}
}

void sniffer_params_init(void) {
	read_config_file();

	midi_set_interface(midi_interface);
	midi_set_baudrate(midi_baudrate);
}
