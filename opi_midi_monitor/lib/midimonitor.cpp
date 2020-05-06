/**
 * @file midimonitor.h
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <stdio.h>

#include "midimonitor.h"

#include "midi.h"
#include "midi_description.h"

#include "hardware.h"
#include "console.h"

#include "h3_hs_timer.h"

#ifndef MIN
 #define MIN(a,b)               (((a) < (b)) ? (a) : (b))
#endif

static uint8_t s_Qf[8] __attribute__ ((aligned (4))) = { 0, 0, 0, 0, 0, 0, 0, 0 };

static char s_aTimecode[] __attribute__ ((aligned (4))) =  "--:--:--;-- -----";
#define TIMECODE_LENGTH	(sizeof(s_aTimecode) - 1)

constexpr char aTypes[4][8] = {"Film " , "EBU  " , "DF   " , "SMPTE" };

#define ROW				1
#define COLUMN			80

inline static void itoa_base10(int nArg, char *pBuffer) {
	char *p = pBuffer;

	if (nArg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = '0' + (nArg / 10);
	*p = '0' + (nArg % 10);
}

MidiMonitor::MidiMonitor(void):
	m_nMillisPrevious(Hardware::Get()->Millis()),
	m_pMidiMessage(midi_message_get()),
	m_nInitTimestamp(0),
	m_nTypePrevious(0xFF),	// Invalid type. Force initial update.
	m_nPartPrevious(0),
	m_bDirection(true)
{
}

MidiMonitor::~MidiMonitor(void) {
}

void MidiMonitor::Init(void) {
	//                                   1         2         3         4
	//                          1234567890123456789012345678901234567890
	const char aHeaderLine[] = "TIMESTAMP ST D1 D2 CHL NOTE EVENT";

	console_set_fg_bg_color(CONSOLE_BLACK,CONSOLE_WHITE);
	console_set_cursor(0, 3);
	console_puts(aHeaderLine);

	for (uint32_t i = sizeof(aHeaderLine); i <= console_get_line_width(); i++) {
		console_putc((int) ' ');
	}

	console_set_fg_bg_color(CONSOLE_WHITE, CONSOLE_BLACK);
	console_set_top_row(4);

	m_nInitTimestamp = h3_hs_timer_lo_us();
}

void MidiMonitor::Update(uint8_t nType) {
	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_write(s_aTimecode, TIMECODE_LENGTH);
	console_restore_cursor();

	if (nType != m_nTypePrevious) {
		m_nTypePrevious = nType;
		memcpy(&s_aTimecode[12], aTypes[nType], 5);
	}
}

void MidiMonitor::HandleMtc(void) {
	const uint8_t type = m_pMidiMessage->system_exclusive[5] >> 5;

	itoa_base10((m_pMidiMessage->system_exclusive[5] & 0x1F), &s_aTimecode[0]);
	itoa_base10(m_pMidiMessage->system_exclusive[6], &s_aTimecode[3]);
	itoa_base10(m_pMidiMessage->system_exclusive[7], &s_aTimecode[6]);
	itoa_base10(m_pMidiMessage->system_exclusive[8], &s_aTimecode[9]);

	Update(type);
}

void MidiMonitor::HandleQf(void) {
	const uint8_t nPart = (m_pMidiMessage->data1 & 0x70) >> 4;
	const uint8_t nValue = m_pMidiMessage->data1 & 0x0F;

	s_Qf[nPart] = nValue;

	if ((nPart == 7) || (m_nPartPrevious == 7)) {
	} else {
		m_bDirection = (m_nPartPrevious < nPart);
	}

	if ((m_bDirection && (nPart == 7)) || (!m_bDirection && (nPart == 0))) {
		itoa_base10(s_Qf[6] | ((s_Qf[7] & 0x1) << 4) , &s_aTimecode[0]);
		itoa_base10(s_Qf[4] | (s_Qf[5] << 4) , &s_aTimecode[3]);
		itoa_base10(s_Qf[2] | (s_Qf[3] << 4) , &s_aTimecode[6]);
		itoa_base10(s_Qf[0] | (s_Qf[1] << 4) , &s_aTimecode[9]);

		const uint8_t nType = s_Qf[7] >> 1;

		Update(nType);
	}

	m_nPartPrevious = nPart;
}

void MidiMonitor::HandleMessage(void) {
	int i;

	if (midi_read_channel(MIDI_CHANNEL_OMNI)) {

		// Handle Active Sensing messages
		if (m_pMidiMessage->type == MIDI_TYPES_ACTIVE_SENSING) {
			// This is handled in ShowActiveSense
			return;
		}

		// Time stamp
		const uint32_t nDeltaUs = m_pMidiMessage->timestamp - m_nInitTimestamp;
		uint32_t nTime = nDeltaUs / 1000;
		const uint32_t nHours = nTime / 3600000;
		nTime -= nHours * 3600000;
		const uint32_t nMinutes = nTime /  60000;
		nTime -= nMinutes * 60000;
		const uint32_t nSeconds = nTime / 1000;
		const uint32_t nMillis = nTime - nSeconds * 1000;

		printf("%02d:%02d.%03d ", (int) (nHours * 60) + nMinutes, (int) nSeconds, (int) nMillis);

		console_puthex(m_pMidiMessage->type);
		console_putc((int) ' ');

		switch (m_pMidiMessage->bytes_count) {
		case 1:
			console_puts("-- -- ");
			break;
		case 2:
			console_puthex(m_pMidiMessage->data1);
			console_puts(" -- ");
			break;
		case 3:
			console_puthex(m_pMidiMessage->data1);
			console_putc((int) ' ');
			console_puthex(m_pMidiMessage->data2);
			console_putc((int) ' ');
			break;
		default:
			console_puts("-- -- ");
			break;
		}

		if (m_pMidiMessage->channel != 0) {
			// Channel messages
			printf("%2d  ", (int) m_pMidiMessage->channel);
			if (m_pMidiMessage->type == MIDI_TYPES_NOTE_OFF || m_pMidiMessage->type == MIDI_TYPES_NOTE_ON) {
				i = console_puts(midi_description_get_key_name(m_pMidiMessage->data1));
				while ((5 - i++) > 0) {
					console_putc((int) ' ');
				}
			}
			else {
				console_puts("---- ");
			}
		} else {
			console_puts("--  ---- ");
		}

		console_puts(midi_description_get_type(m_pMidiMessage->type));

		if (m_pMidiMessage->channel != 0) {
			// Channel messages
			switch (m_pMidiMessage->type) {
			// Channel message
			case MIDI_TYPES_NOTE_OFF:
			case MIDI_TYPES_NOTE_ON:
				printf(" %d, Velocity %d\n",(int) m_pMidiMessage->data1, (int) m_pMidiMessage->data2);
				break;
			case MIDI_TYPES_AFTER_TOUCH_POLY:
				printf(" %d, Pressure %d\n", (int) m_pMidiMessage->data1, (int) m_pMidiMessage->data2);
				break;
			case MIDI_TYPES_CONTROL_CHANGE:
				// https://www.midi.org/specifications/item/table-3-control-change-messages-data-bytes-2
				if (m_pMidiMessage->data1 < 120) {
					// Control Change
					printf(", %s, Value %d\n", midi_description_get_control_function(m_pMidiMessage->data1), (int) m_pMidiMessage->data2);
				} else {
					// Controller numbers 120-127 are reserved for Channel Mode Messages, which rather than controlling sound parameters, affect the channel's operating mode.
					// Channel Mode Messages
					printf(", %s", midi_description_get_control_change(m_pMidiMessage->data1));

					if (m_pMidiMessage->data1	== MIDI_CONTROL_CHANGE_LOCAL_CONTROL) {
						printf(" %s\n",	m_pMidiMessage->data2 == 0 ? "OFF" : "ON");
					} else {
						console_putc((int) '\n');
					}
				}
				break;
			case MIDI_TYPES_PROGRAM_CHANGE:
				if (m_pMidiMessage->channel == 10) {
					printf(", %s {%d}\n", midi_description_get_drum_kit_name(m_pMidiMessage->data1), (int) m_pMidiMessage->data1);
				} else {
					printf(", %s {%d}\n", midi_description_get_instrument_name(m_pMidiMessage->data1), (int) m_pMidiMessage->data1);
				}
				break;
			case MIDI_TYPES_AFTER_TOUCH_CHANNEL:
				printf(", Pressure %d\n", (int) m_pMidiMessage->data1);
				break;
			case MIDI_TYPES_PITCH_BEND:
				printf(", Bend %d\n", (int) (m_pMidiMessage->data1 | (m_pMidiMessage->data2 << 7)));
				break;
			default:
				break;
			}
		} else {
			switch (m_pMidiMessage->type) {
			// 1 byte message
			case MIDI_TYPES_START:
			case MIDI_TYPES_CONTINUE:
			case MIDI_TYPES_STOP:
			case MIDI_TYPES_CLOCK:
			case MIDI_TYPES_ACTIVE_SENSING:
			case MIDI_TYPES_SYSTEM_RESET:
			case MIDI_TYPES_TUNE_REQUEST:
				console_putc((int) '\n');
				break;
				// 2 bytes messages
			case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
				printf(", Message number %d, Data %d\n", (int) ((m_pMidiMessage->data1 & 0x70) >> 4), (int) (m_pMidiMessage->data1 & 0x0F));
				HandleQf();
				break;
			case MIDI_TYPES_SONG_SELECT:
				printf(", Song id number %d\n",(int) m_pMidiMessage->data1);
				break;
				// 3 bytes messages
			case MIDI_TYPES_SONG_POSITION:
				printf(", Song position %d\n", (int) (m_pMidiMessage->data1 | (m_pMidiMessage->data2 << 7)));
				break;
				// > 3 bytes messages
			case MIDI_TYPES_SYSTEM_EXCLUSIVE:
				printf(", [%d] ", (int) m_pMidiMessage->bytes_count);
				{
					uint8_t c;
					for (c = 0; c < MIN(m_pMidiMessage->bytes_count, 16); c++) {
						console_puthex(m_pMidiMessage->system_exclusive[c]);
						console_putc(' ');
					}
					if (c < m_pMidiMessage->bytes_count) {
						console_puts("..");
					}
				}
				console_putc('\n');
				if ((m_pMidiMessage->system_exclusive[1] == 0x7F) && (m_pMidiMessage->system_exclusive[2] == 0x7F) && (m_pMidiMessage->system_exclusive[3] == 0x01)) {
					HandleMtc();
				}
				break;
			case MIDI_TYPES_INVALIDE_TYPE:
			default:
				console_puts(", Invalid MIDI message\n");
				break;
			}
		}
	}
}

void MidiMonitor::ShowActiveSense(void) {
	uint32_t nNow = Hardware::Get()->Millis();

	if (__builtin_expect(((nNow - m_nMillisPrevious) < 1000), 0)) {
		return;
	}

	m_nMillisPrevious = nNow;

	const _midi_active_sense_state tState = midi_active_get_sense_state();

	if (tState == MIDI_ACTIVE_SENSE_ENABLED) {
		console_save_cursor();
		console_set_cursor(70, 3);
		console_set_fg_bg_color(CONSOLE_BLACK, CONSOLE_CYAN);
		console_puts("ACTIVE SENSING          ");
		console_restore_cursor();
	} else if (tState == MIDI_ACTIVE_SENSE_FAILED) {
		console_save_cursor();
		console_set_cursor(70, 3);
		console_set_fg_bg_color(CONSOLE_RED, CONSOLE_WHITE);
		console_puts("ACTIVE SENSING - Failed!");
		console_restore_cursor();
	}
}

void MidiMonitor::Run(void) {
	HandleMessage();
	ShowActiveSense();
}

