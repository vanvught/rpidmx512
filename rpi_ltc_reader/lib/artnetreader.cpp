/**
 * @file artnetreader.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>

#include "c/hardware.h"

#include "arm/synchronize.h"
#include "arm/irq_timer.h"
#include "bcm2835.h"

#include "ltc_reader.h"

#include "artnetreader.h"
#include "artnettimecode.h"

#include "console.h"
#include "lcd.h"
#include "display_oled.h"
#include "display_7segment.h"
#include "display_matrix.h"

#include "midi.h"

#include "util.h"

static volatile char timecode[TC_CODE_MAX_LENGTH] ALIGNED;

static volatile uint32_t updates_per_second= (uint32_t) 0;
static volatile uint32_t updates_previous = (uint32_t) 0;
static volatile uint32_t updates = (uint32_t) 0;

static volatile uint32_t led_counter = (uint32_t) 0;

static struct _midi_send_tc midi_timecode = { 0, 0, 0, 0, MIDI_TC_TYPE_EBU };

static volatile uint32_t midi_quarter_frame_us = (uint32_t) 0;
static volatile bool midi_quarter_frame_message = false;
static volatile uint8_t midi_quarter_frame_piece ALIGNED = 0;

static void irq_timer1_update_handler(const uint32_t clo) {
	BCM2835_ST->C1 = clo + (uint32_t) 1000000;

	dmb();
	updates_per_second = updates - updates_previous;
	updates_previous = updates;

	if ((updates_per_second >= 24) && (updates_per_second <= 30)) {
		hardware_led_set((int) (led_counter++ & 0x01));
	} else {
		hardware_led_set(0);
	}
}

static void irq_timer3_midi_handler(const uint32_t clo) {
	BCM2835_ST->C3 = clo + midi_quarter_frame_us;

	dmb();
	midi_quarter_frame_message = true;
}

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

ArtNetReader::ArtNetReader(void) : m_pOutput(0) , m_PrevType(TC_TYPE_INVALID) {
	for (unsigned i = 0; i < sizeof(timecode) / sizeof(timecode[0]) ; i++) {
		timecode[i] = ' ';
	}

	timecode[2] = ':';
	timecode[5] = ':';
	timecode[8] = '.';
}

ArtNetReader::~ArtNetReader(void) {
	this->Stop();
}

void ArtNetReader::Start(const struct _ltc_reader_output *output) {
	m_pOutput = (struct _ltc_reader_output *) output;

	irq_timer_init();
	irq_timer_set(IRQ_TIMER_1, irq_timer1_update_handler);
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;
	irq_timer_set(IRQ_TIMER_3, irq_timer3_midi_handler);
	BCM2835_ST->C3 = BCM2835_ST->CLO;
	dmb();

	Start();
}

void ArtNetReader::Start(void) {
	if (m_pOutput == NULL) {
		return;
	}

	if (m_pOutput->lcd_output) {
		lcd_cls();
	}
}

void ArtNetReader::Stop(void) {
	char *p;

	if (m_pOutput == NULL) {
		return;
	}

	p = (char *) ltc_reader_get_type(TC_TYPE_UNKNOWN);

	if (m_pOutput->console_output) {
		console_set_cursor(2, 25);
		(void) console_puts(p);
	}

	if (m_pOutput->lcd_output) {
		lcd_text_line_2(p, TC_TYPE_MAX_LENGTH);
	}

	if (m_pOutput->oled_output) {
		display_oled_line_2(p);
	}
}

void ArtNetReader::Handler(const struct TArtNetTimeCode *ArtNetTimeCode) {
	char sLimitWarning[16];
	uint32_t nLimitUs = 0;
	char *p_type;

	if (m_pOutput == NULL) {
		return;
	}

	const uint32_t nNowUs = BCM2835_ST->CLO;

	updates++;

	itoa_base10(ArtNetTimeCode->Hours, (char *) &timecode[0]);
	itoa_base10(ArtNetTimeCode->Minutes, (char *) &timecode[3]);
	itoa_base10(ArtNetTimeCode->Seconds, (char *) &timecode[6]);
	itoa_base10(ArtNetTimeCode->Frames, (char *) &timecode[9]);

	midi_timecode.hour = ArtNetTimeCode->Hours;
	midi_timecode.minute = ArtNetTimeCode->Minutes;
	midi_timecode.second = ArtNetTimeCode->Seconds;
	midi_timecode.frame = ArtNetTimeCode->Frames;
	midi_timecode.rate = (_midi_timecode_type) ArtNetTimeCode->Type;

	switch ((_midi_timecode_type) ArtNetTimeCode->Type) {
	case TC_TYPE_FILM:
		nLimitUs = (uint32_t) ((double) 1000000 / (double) 24);
		break;
	case TC_TYPE_EBU:
		nLimitUs = (uint32_t) ((double) 1000000 / (double) 25);
		break;
	case TC_TYPE_DF:
	case TC_TYPE_SMPTE:
		nLimitUs = (uint32_t) ((double) 1000000 / (double) 30);
		break;
	default:
		nLimitUs = 0;
		break;
	}

	if (m_pOutput->console_output) {
		console_set_cursor(2, 24);
		console_write((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if (m_pOutput->lcd_output) {
		lcd_text_line_1((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if(m_pOutput->oled_output) {
		display_oled_line_1((char *) timecode, TC_CODE_MAX_LENGTH);
	}

	if (m_pOutput->segment_output) {
		display_7segment((const char *) timecode);
	}

	if (m_pOutput->matrix_output) {
		display_matrix((const char *) timecode);
	}

	if ((m_PrevType != ArtNetTimeCode->Type)) {
		p_type = (char *) ltc_reader_get_type((timecode_types) ArtNetTimeCode->Type);
		m_PrevType = ArtNetTimeCode->Type;

		if (m_pOutput->midi_output) {
			midi_send_tc((struct _midi_send_tc *)&midi_timecode);

			midi_quarter_frame_piece = 0;
			midi_quarter_frame_us = nLimitUs / (uint32_t) 4;
			BCM2835_ST->C3 = nNowUs + midi_quarter_frame_us;
		}

		if (m_pOutput->console_output) {
			console_set_cursor(2, 25);
			(void) console_puts(p_type);
		}

		if (m_pOutput->lcd_output) {
			lcd_text_line_2(p_type, TC_TYPE_MAX_LENGTH);
		}

		if (m_pOutput->oled_output) {
			display_oled_line_2(p_type);
		}
	}

	const uint32_t nDeltaUs = BCM2835_ST->CLO - nNowUs;

	if (nLimitUs == 0) {
		sprintf(sLimitWarning, "%.2d:-----:%.5d", (int) updates_per_second, (int) nDeltaUs);
		console_status(CONSOLE_CYAN, sLimitWarning);
	} else {
		sprintf(sLimitWarning, "%.2d:%.5d:%.5d", (int) updates_per_second, (int) nLimitUs, (int) nDeltaUs);
		console_status(nDeltaUs < nLimitUs ? CONSOLE_YELLOW : CONSOLE_RED, sLimitWarning);
	}
}

void ArtNetReader::Run(void) {
	if ((m_pOutput->midi_output) && (updates_per_second >= 24) && (updates_per_second <= 30)) {
		dmb();
		if (midi_quarter_frame_message) {
			dmb();
			midi_quarter_frame_message = false;

			uint8_t bytes[2] = { 0xF1, 0x00 };
			uint8_t data = midi_quarter_frame_piece << 4;

			switch (midi_quarter_frame_piece) {
			case 0:
				bytes[1] = data | (midi_timecode.frame & 0x0F);
				break;
			case 1:
				bytes[1] = data | ((midi_timecode.frame & 0x10) >> 4);
				break;
			case 2:
				bytes[1] = data | (midi_timecode.second & 0x0F);
				break;
			case 3:
				bytes[1] = data | ((midi_timecode.second & 0x30) >> 4);
				break;
			case 4:
				bytes[1] = data | (midi_timecode.minute & 0x0F);
				break;
			case 5:
				bytes[1] = data | ((midi_timecode.minute & 0x30) >> 4);
				break;
			case 6:
				bytes[1] = data | (midi_timecode.hour & 0x0F);
				break;
			case 7:
				bytes[1] = data | (midi_timecode.rate << 1) | ((midi_timecode.hour & 0x10) >> 4);
				break;
			default:
				break;
			}

			midi_send_raw(bytes, 2);
			midi_quarter_frame_piece = (midi_quarter_frame_piece + (uint8_t) 1) & (uint8_t) 0x07;
		}
	}
}
