/**
 * @file sniffer.c
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sniffer_params.h"
#include "midi.h"
#include "midi_description.h"

#include "console.h"

static const struct _midi_message *midi_message;

void sniffer_midi(void) {
	int i;

	if (midi_read_channel(MIDI_CHANNEL_OMNI)) {
		// Time stamp
		(void) console_puts("........  ");
		console_puthex(midi_message->type);
		(void) console_putc((int) ' ');

		switch (midi_message->bytes_count) {
			case 1:
				(void) console_puts("-- -- ");
				break;
			case 2:
				console_puthex(midi_message->data1);
				(void) console_puts(" -- ");
				break;
			case 3:
				console_puthex(midi_message->data1);
				(void) console_putc((int) ' ');
				console_puthex(midi_message->data2);
				(void) console_putc((int) ' ');
				break;
			default:
				(void) console_puts("-- -- ");
				break;
		}

		if (midi_message->channel != 0) {
			// Channel messages
			printf("%2d  ", (int) midi_message->channel);
			if (midi_message->type == MIDI_TYPES_NOTE_OFF || midi_message->type == MIDI_TYPES_NOTE_ON) {
				i = console_puts(midi_description_get_key_name(midi_message->data1));
				while ((5 - i++) > 0) {
					(void) console_putc((int) ' ');
				}
			}
			else {
				(void) console_puts("---- ");
			}
		} else {
			(void) console_puts("--  ---- ");
		}

		(void) console_puts(midi_description_get_type(midi_message->type));

		if (midi_message->channel != 0) {
			// Channel messages
			switch (midi_message->type) {
			// Channel message
			case MIDI_TYPES_NOTE_OFF:
			case MIDI_TYPES_NOTE_ON:
				printf(" %d, Velocity %d\n",(int) midi_message->data1, (int) midi_message->data2);
				break;
			case MIDI_TYPES_AFTER_TOUCH_POLY:
				printf(" %d, Pressure %d\n", (int) midi_message->data1, (int) midi_message->data2);
				break;
			case MIDI_TYPES_CONTROL_CHANGE:
				// https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
				if (midi_message->data1 < 120) {
					// Control Change
					printf(", %s, Value %d\n", midi_description_get_control_function(midi_message->data1), (int) midi_message->data2);
				} else {
					// Controller numbers 120-127 are reserved for Channel Mode Messages, which rather than controlling sound parameters, affect the channel's operating mode.
					// Channel Mode Messages
					printf(", %s", midi_description_get_control_change(midi_message->data1));

					if (midi_message->data1	== MIDI_CONTROL_CHANGE_LOCAL_CONTROL) {
						printf(" %s\n",	midi_message->data2 == 0 ? "OFF" : "ON");
					} else {
						(void) console_putc((int) '\n');
					}
				}
				break;
			case MIDI_TYPES_PROGRAM_CHANGE:
				if (midi_message->channel == 10) {
					printf(", %s {%d}\n", midi_description_get_drum_kit_name(midi_message->data1), (int) midi_message->data1);
				} else {
					printf(", %s {%d}\n", midi_description_get_instrument_name(midi_message->data1), (int) midi_message->data1);
				}
				break;
			case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
				printf(", Pressure %d\n", (int) midi_message->data1);
				break;
			case MIDI_TYPES_PITCH_BEND:
				printf(", Bend %d\n", (int) (midi_message->data1 | (midi_message->data2 << 7)));
				break;
			default:
				break;
			}
		} else {
			switch (midi_message->type) {
			// 1 byte message
			case MIDI_TYPES_START:
			case MIDI_TYPES_CONTINUE:
			case MIDI_TYPES_STOP:
			case MIDI_TYPES_CLOCK:
			case MIDI_TYPES_ACTIVE_SENSING:
			case MIDI_TYPES_SYSTEM_RESET:
			case MIDI_TYPES_TUNE_REQUEST:
				(void) console_putc((int) '\n');
				break;
				// 2 bytes messages
			case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
				printf(", Message number %d, Data %d\n", (int) ((midi_message->data1 & 0x70) >> 4), (int) (midi_message->data1 & 0x0F));
				break;
			case MIDI_TYPES_SONG_SELECT:
				printf(", Song id number %d\n",(int) midi_message->data1);
				break;
				// 3 bytes messages
			case MIDI_TYPES_SONG_POSITION:
				printf(", Song position %d\n", (int) (midi_message->data1 | (midi_message->data2 << 7)));
				break;
				// > 3 bytes messages
			case MIDI_TYPES_SYSTEM_EXCLUSIVE:
				(void) console_putc((int) '\n'); //TODO Not implemented, yet
				break;
			case MIDI_TYPES_INVALIDE_TYPE:
			default:
				(void) console_puts(", Invalid MIDI message\n");
				break;
			}
		}
	}
}

void sniffer_init(void) {
	int i;
	//                                   1         2         3         4
	//                          1234567890123456789012345678901234567890
	const char header_line[] = "TIMESTAMP ST D1 D2 CHL NOTE EVENT";

	console_set_bg_color(CONSOLE_WHITE);
	console_set_fg_color(CONSOLE_BLACK);

	console_set_cursor(0, 3);

	(void) console_puts(header_line);

	for (i = (int) sizeof(header_line); i <= console_get_line_width(); i++) {
		(void) console_putc((int) ' ');
	}

	console_set_bg_color(CONSOLE_BLACK);
	console_set_fg_color(CONSOLE_WHITE);

	console_set_top_row(4);

	midi_message = (const struct _midi_message *) midi_message_get();
}
