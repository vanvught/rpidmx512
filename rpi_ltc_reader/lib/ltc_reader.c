/**
 * @file ltc_reader.c
 *
 */
/*
 * Parts of this code is inspired by:
 * http://forum.arduino.cc/index.php/topic,8237.0.html
 *
 * References :
 * https://en.wikipedia.org/wiki/Linear_timecode
 * http://www.philrees.co.uk/articles/timecode.htm
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
#include <stdio.h>
#include <assert.h>

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_st.h"
#include "bcm2835_gpio.h"

#include "hardware.h"

#include "gpio.h"

#include "midi.h"

#include "ltc_reader.h"
#include "ltc_reader_params.h"

#include "console.h"
#include "lcd.h"
#include "display_oled.h"
#include "display_7segment.h"
#include "display_matrix.h"

#include "util.h"

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	380	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63	///<
#define END_SYNC_POSITION	77	///<
#define END_SMPTE_POSITION	80	///<

static volatile char timecode[TC_CODE_MAX_LENGTH] ALIGNED;

static const struct _ltc_reader_output *output;

static timecode_types prev_type = TC_TYPE_INVALID;	///< Invalid type. Force initial update.

static volatile uint32_t fiq_us_previous = 0;
static volatile uint32_t fiq_us_current = 0;

static volatile uint32_t bit_time = 0;
static volatile uint32_t total_bits = 0;
static volatile bool ones_bit_count = false;
static volatile uint32_t current_bit = 0;
static volatile uint32_t sync_count = 0;
static volatile bool timecode_sync = false;
static volatile bool timecode_valid = false;

static volatile uint8_t timecode_bits[8] ALIGNED;
static volatile bool is_drop_frame_flag_set = false;

static volatile bool timecode_available = false;

static volatile uint32_t ltc_updates_per_second= (uint32_t) 0;
static volatile uint32_t ltc_updates_previous = (uint32_t) 0;
static volatile uint32_t ltc_updates = (uint32_t) 0;

static volatile uint32_t led_counter = (uint32_t) 0;

static volatile struct _midi_send_tc midi_timecode = { 0, 0, 0, 0, MIDI_TC_TYPE_EBU };

static volatile uint32_t midi_quarter_frame_us = (uint32_t) 0;
static volatile bool midi_quarter_frame_message = false;
static volatile uint8_t midi_quarter_frame_piece ALIGNED = 0;

extern void artnet_output(const struct _midi_send_tc *);

/**
 *
 */
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();

	const uint32_t clo = BCM2835_ST->CLO;

	if (BCM2835_ST->CS & BCM2835_ST_CS_M1) {
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		BCM2835_ST->C1 = clo + (uint32_t) 1000000;

		dmb();
		ltc_updates_per_second = ltc_updates - ltc_updates_previous;
		ltc_updates_previous = ltc_updates;

		if ((ltc_updates_per_second >= 24) && (ltc_updates_per_second <= 30)) {
			hardware_led_set((int) (led_counter++ & 0x01));
		} else {
			hardware_led_set(0);
		}

	}
	
	if (BCM2835_ST->CS & BCM2835_ST_CS_M3) {
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		BCM2835_ST->C3 = clo + midi_quarter_frame_us;

		dmb();
		midi_quarter_frame_message = true;
	}

	dmb();
}

/**
 *
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();
	fiq_us_current = BCM2835_ST->CLO;

	BCM2835_GPIO->GPEDS0 = 1 << GPIO_PIN;

	dmb();
	bit_time = fiq_us_current - fiq_us_previous;

	if ((bit_time < ONE_TIME_MIN) || (bit_time > ZERO_TIME_MAX)) {
		total_bits = 0;
	} else {
		if (ones_bit_count) {
			ones_bit_count = false;
		} else {
			if (bit_time > ZERO_TIME_MIN) {
				current_bit = 0;
				sync_count = 0;
			} else {
				current_bit = 1;
				ones_bit_count = true;
				sync_count++;

				if (sync_count == 12) {
					sync_count = 0;
					timecode_sync = true;
					total_bits = END_SYNC_POSITION;
				}
			}

			if (total_bits <= END_DATA_POSITION) {
				timecode_bits[0] = timecode_bits[0] >> 1;
				int n;
				for (n = 1; n < 8; n++) {
					if (timecode_bits[n] & 1) {
						timecode_bits[n - 1] |= (uint8_t) 0x80;
					}
					timecode_bits[n] = timecode_bits[n] >> 1;
				}

				if (current_bit == 1) {
					timecode_bits[7] |= (uint8_t) 0x80;
				}
			}

			total_bits++;
		}

		if (total_bits == END_SMPTE_POSITION) {

			total_bits = 0;

			if (timecode_sync) {
				timecode_sync = false;
				timecode_valid = true;
			}
		}

		if (timecode_valid) {
			dmb();
			ltc_updates++;

			timecode_valid = false;

			midi_timecode.frame  = (10 * (timecode_bits[1] & 0x03)) + (timecode_bits[0] & 0x0F);
			midi_timecode.second = (10 * (timecode_bits[3] & 0x07)) + (timecode_bits[2] & 0x0F);
			midi_timecode.minute = (10 * (timecode_bits[5] & 0x07)) + (timecode_bits[4] & 0x0F);
			midi_timecode.hour   = (10 * (timecode_bits[7] & 0x03)) + (timecode_bits[6] & 0x0F);

			timecode[10] = (timecode_bits[0] & 0x0F) + '0';	// frames
			timecode[9]  = (timecode_bits[1] & 0x03) + '0';	// 10's of frames
			timecode[7]  = (timecode_bits[2] & 0x0F) + '0';	// seconds
			timecode[6]  = (timecode_bits[3] & 0x07) + '0';	// 10's of seconds
			timecode[4]  = (timecode_bits[4] & 0x0F) + '0';	// minutes
			timecode[3]  = (timecode_bits[5] & 0x07) + '0';	// 10's of minutes
			timecode[1]  = (timecode_bits[6] & 0x0F) + '0';	// hours
			timecode[0]  = (timecode_bits[7] & 0x03) + '0';	// 10's of hours

			is_drop_frame_flag_set = (timecode_bits[1] & (1 << 2));

			dmb();
			timecode_available = true;
		}
	}

	fiq_us_previous = fiq_us_current;
	dmb();
}

/**
 *
 */
void ltc_reader(void) {
	uint8_t type = TC_TYPE_UNKNOWN;
	uint32_t limit_us = (uint32_t) 0;
	uint32_t now_us = (uint32_t) 0;
	char limit_warning[16] ALIGNED;
	char *p_type;

	dmb();
	if (timecode_available) {
		dmb();
		timecode_available = false;

		now_us = BCM2835_ST->CLO;

		type = TC_TYPE_UNKNOWN;

		dmb();
		if (is_drop_frame_flag_set) {
			type = TC_TYPE_DF;
			limit_us = (uint32_t)((double)1000000 / (double)30);
		} else {
			if (ltc_updates_per_second == 24) {
				type = TC_TYPE_FILM;
				limit_us = (uint32_t)((double)1000000 / (double)24);
			} else if (ltc_updates_per_second == 25) {
				type = TC_TYPE_EBU;
				limit_us = (uint32_t)((double)1000000 / (double)25);
			} else if (ltc_updates_per_second == 30) {
				limit_us = (uint32_t)((double)1000000 / (double)30);
				type = TC_TYPE_SMPTE;
			}
		}

		midi_timecode.rate = type;

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

		if (output->artnet_output) {
			artnet_output((struct _midi_send_tc *)&midi_timecode);
		}

		if (output->matrix_output) {
			display_matrix((const char *) timecode);
		}

		if (prev_type != type) {
			p_type = (char *) ltc_reader_get_type((timecode_types) type);
			prev_type = type;

			if (output->midi_output) {
				midi_send_tc((struct _midi_send_tc *)&midi_timecode);

				midi_quarter_frame_piece = 0;
				midi_quarter_frame_us = limit_us / (uint32_t) 4;
				BCM2835_ST->C3 = now_us + midi_quarter_frame_us;
			}

			if (output->console_output) {
				console_set_cursor(2, 25);
				console_puts(p_type);
			}

			if (output->lcd_output) {
				lcd_text_line_2(p_type, TC_TYPE_MAX_LENGTH);
			}

			if (output->oled_output) {
				display_oled_line_2(p_type);
			}

		}

		const uint32_t delta_us = BCM2835_ST->CLO - now_us;

		if (limit_us == 0) {
			sprintf(limit_warning, "%.2d:-----:%.5d", (int) ltc_updates_per_second, (int) delta_us);
			console_status(CONSOLE_CYAN, limit_warning);
		} else {
			sprintf(limit_warning, "%.2d:%.5d:%.5d", (int) ltc_updates_per_second, (int) limit_us, (int) delta_us);
			console_status(delta_us < limit_us ? CONSOLE_YELLOW : CONSOLE_RED, limit_warning);
		}
	}

	if ((output->midi_output) && (ltc_updates_per_second >= 24) && (ltc_updates_per_second <= 30)) {
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
					bytes[1] = data | (midi_timecode.rate << 1) |((midi_timecode.hour & 0x10) >> 4);;
					break;
				default:
					break;
			}

			midi_send_raw(bytes, 2);
			midi_quarter_frame_piece = (midi_quarter_frame_piece + (uint8_t) 1) & (uint8_t) 0x07;
		}
	}
}

/**
 *
 */
void ltc_reader_init(const struct _ltc_reader_output *out) {
	int i;
	
	assert(out != NULL);

	output = out;

	bcm2835_gpio_fsel(GPIO_PIN, BCM2835_GPIO_FSEL_INPT);
	// Rising Edge
	BCM2835_GPIO->GPREN0 = 1 << GPIO_PIN;
	// Falling Edge
	BCM2835_GPIO->GPFEN0 = 1 << GPIO_PIN;
	// Clear status bit
	BCM2835_GPIO->GPEDS0 = 1 << GPIO_PIN;
	// Enable GPIO FIQ
	BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | INTERRUPT_GPIO0;

	dmb();
	__enable_fiq();

	BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;
	BCM2835_ST->C3 = BCM2835_ST->CLO;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn + BCM2835_TIMER3_IRQn;

	dmb();
	__enable_irq();

	for (i = 0; i < sizeof(timecode) / sizeof(timecode[0]) ; i++) {
		timecode[i] = ' ';
	}

	timecode[2] = ':';
	timecode[5] = ':';
	timecode[8] = '.';

	if (output->lcd_output) {
		lcd_cls();
	}
}
