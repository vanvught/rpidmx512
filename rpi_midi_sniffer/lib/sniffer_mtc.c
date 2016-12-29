/**
 * @file sniffer_mtc.c
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

#include "midi.h"

#include "console.h"
#include "util.h"

#define ROW		1		///<
#define COLUMN	80		///<

static char timecode[] ALIGNED =  "--:--:--;-- -----";
#define TIMECODE_LENGTH	((sizeof(timecode) / sizeof(char)) - 1)

static const char types[4][8] ALIGNED = {"Film " , "EBU  " , "DF   " , "SMPTE" };

static uint8_t prev_type = 0xFF;	///< Invalid type. Force initial update.

static void itoa_base10(int arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = (char) '0' + (char) (arg / 10);
	*n = (char) '0' + (char) (arg % 10);
}

/**
 *
 * @param midi_message
 */
void sniffer_tmc(const struct _midi_message *midi_message) {
	const uint8_t type = midi_message->system_exclusive[5] >> 5;

	itoa_base10((midi_message->system_exclusive[5] & 0x1F), (char *) &timecode[0]);
	itoa_base10(midi_message->system_exclusive[6], (char *) &timecode[3]);
	itoa_base10(midi_message->system_exclusive[7], (char *) &timecode[6]);
	itoa_base10(midi_message->system_exclusive[8], (char *) &timecode[9]);

	if (type != prev_type) {
		memcpy((char *) &timecode[12], (char *) types[type], 5);
		prev_type = type;
	}

	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_write(timecode, TIMECODE_LENGTH);
	console_restore_cursor();

}
