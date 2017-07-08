/**
 * @file midi_reader_mtc.c
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

#include "midi.h"
#include "midi_params.h"
#include "midi_description.h"

#include "ltc_reader.h"

#include "console.h"
#include "lcd.h"
#include "display_oled.h"
#include "display_7segment.h"
#include "display_matrix.h"

extern void artnet_output(const struct _midi_send_tc *);

static const struct _ltc_reader_output *output;

static volatile char timecode[TC_CODE_MAX_LENGTH] ALIGNED;
static struct _midi_send_tc midi_timecode = { 0, 0, 0, 0, MIDI_TC_TYPE_EBU };

static uint8_t prev_type ALIGNED = 0xFF;					///< Invalid type. Force initial update.
static uint8_t qf[8] ALIGNED = { 0, 0, 0, 0, 0, 0, 0, 0 };	///<
static uint8_t prev_part ALIGNED = 0;						///<

static void itoa_base10(uint8_t arg, char *buf) {
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
 * @param type
 */
static void update(const uint8_t type) {
	char *p_type;

	if (output->artnet_output) {
		artnet_output((struct _midi_send_tc *)&midi_timecode);
	}

	if (output->console_output) {
		console_set_cursor(2, 24);
		console_write((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if (output->lcd_output) {
		lcd_text_line_1((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if(output->oled_output) {
		display_oled_line_1((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if (output->segment_output) {
		display_7segment((const char *) timecode);
	}

	if (output->matrix_output) {
		display_matrix((const char *) timecode);
	}

	if (type != prev_type) {
		p_type = (char *) ltc_reader_get_type((timecode_types) type);
		prev_type = type;

		if (output->console_output) {
			console_set_cursor(2, 25);
			(void) console_puts(p_type);
		}

		if (output->lcd_output) {
			lcd_text_line_2(p_type, TC_TYPE_MAX_LENGTH);
		}

		if (output->oled_output) {
			display_oled_line_2(p_type);
		}
	}
}

/**
 *
 * @param midi_message
 */
const _midi_timecode_type midi_reader_mtc(const struct _midi_message *midi_message) {
	const uint8_t type = midi_message->system_exclusive[5] >> 5;

	itoa_base10((midi_message->system_exclusive[5] & 0x1F), (char *) &timecode[0]);
	itoa_base10(midi_message->system_exclusive[6], (char *) &timecode[3]);
	itoa_base10(midi_message->system_exclusive[7], (char *) &timecode[6]);
	itoa_base10(midi_message->system_exclusive[8], (char *) &timecode[9]);

	midi_timecode.hour = midi_message->system_exclusive[5] & 0x1F;
	midi_timecode.minute = midi_message->system_exclusive[6];
	midi_timecode.second = midi_message->system_exclusive[7];
	midi_timecode.frame = midi_message->system_exclusive[8];
	midi_timecode.rate = (_midi_timecode_type) type;

	update(type);

	return (_midi_timecode_type) type;
}

/**
 *
 * @param midi_message
 */
const _midi_timecode_type midi_reader_mtc_qf(const struct _midi_message *midi_message) {
	uint8_t type = 0;
	const uint8_t part = (midi_message->data1 & 0x70) >> 4;
	const uint8_t value = midi_message->data1 & 0x0F;

	qf[part] = value;
	type = qf[7] >> 1;

	if (((part == 7) && (prev_part == 6)) || ((part == 0) && (prev_part == 7)))  {
		itoa_base10(qf[6] | ((qf[7] & 0x1) << 4) , (char *) &timecode[0]);
		itoa_base10(qf[4] | (qf[5] << 4) , (char *) &timecode[3]);
		itoa_base10(qf[2] | (qf[3] << 4) , (char *) &timecode[6]);
		itoa_base10(qf[0] | (qf[1] << 4) , (char *) &timecode[9]);

		update(type);
	}

	prev_part = part;

	return (_midi_timecode_type) type;
}

void midi_reader_mtc_init(const struct _ltc_reader_output *out) {
	unsigned i;

	for (i = 0; i < sizeof(timecode) / sizeof(timecode[0]) ; i++) {
		timecode[i] = ' ';
	}

	timecode[2] = ':';
	timecode[5] = ':';
	timecode[8] = '.';

	if (output->lcd_output) {
		lcd_cls();
	}

	output = out;
}

