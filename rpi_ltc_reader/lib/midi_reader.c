/**
 * @file midi_reader.c
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

#include <arm/irq_timer.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "bcm2835.h"
#include "arm/synchronize.h"

#include "midi.h"
#include "midi_params.h"
#include "midi_mtc.h"

#include "ltc_reader.h"

#include "console.h"

#include "c/hardware.h"

static const struct _midi_message *midi_message;

extern void midi_poll(void);	///< Needed for SPI interface only.

static volatile uint32_t updates_per_second= (uint32_t) 0;
static volatile uint32_t updates_previous = (uint32_t) 0;
static volatile uint32_t updates = (uint32_t) 0;

static volatile uint32_t led_counter = (uint32_t) 0;

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

void midi_reader(void) {
	uint32_t limit_us = (uint32_t) 0;
	uint32_t now_us = (uint32_t) 0;
	char limit_warning[16];
	bool is_mtc = false;
	uint32_t delta_us;
	_midi_timecode_type type;

	now_us = BCM2835_ST->CLO;

	midi_poll();

	if (midi_read_channel(MIDI_CHANNEL_OMNI)) {
		if (midi_message->channel == 0) {
			switch (midi_message->type) {
			case MIDI_TYPES_TIME_CODE_QUARTER_FRAME:
				type = midi_reader_mtc_qf(midi_message);
				is_mtc = true;
				break;
			case MIDI_TYPES_SYSTEM_EXCLUSIVE:
				if ((midi_message->system_exclusive[1] == 0x7F) && (midi_message->system_exclusive[2] == 0x7F) && (midi_message->system_exclusive[3] == 0x01)) {
					type = midi_reader_mtc(midi_message);
					is_mtc = true;
				}
				break;
			default:
				break;
			}
		}
	}

	if (is_mtc) {
		updates++;

		switch (type) {
		case TC_TYPE_FILM:
			limit_us = (uint32_t) ((double) 1000000 / (double) 24);
			break;
		case TC_TYPE_EBU:
			limit_us = (uint32_t) ((double) 1000000 / (double) 25);
			break;
		case TC_TYPE_DF:
		case TC_TYPE_SMPTE:
			limit_us = (uint32_t) ((double) 1000000 / (double) 30);
			break;
		default:
			limit_us = 0;
			break;
		}

		delta_us = BCM2835_ST->CLO - now_us;

		if (limit_us == 0) {
			sprintf(limit_warning, "%.2d:-----:%.5d", (int) updates_per_second, (int) delta_us);
			console_status(CONSOLE_CYAN, limit_warning);
		} else {
			sprintf(limit_warning, "%.2d:%.5d:%.5d", (int) updates_per_second, (int) limit_us, (int) delta_us);
			console_status(delta_us < limit_us ? CONSOLE_YELLOW : CONSOLE_RED, limit_warning);
		}
	}
}

void midi_reader_init(const struct _ltc_reader_output *out) {
	assert(out != NULL);

	midi_params_init();
	midi_set_interface(midi_params_get_interface());
	midi_set_baudrate(midi_params_get_baudrate());
	midi_active_set_sense(true);
	midi_init(MIDI_DIRECTION_INPUT);

	midi_message = (const struct _midi_message *) midi_message_get();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_update_handler);
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;

	midi_reader_mtc_init(out);
}
