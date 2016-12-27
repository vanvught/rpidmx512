/**
 * @file main.c
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

#include <stdio.h>
#include <stdbool.h>

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"

#include "hardware.h"
#include "console.h"
#include "util.h"

#include "software_version.h"

static const char types[4][8] ALIGNED = {"Film " , "EBU  " , "DF   " , "SMPTE" };
static uint8_t prev_type = 0xFF;	///< Invalid type. Force initial update.

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	400	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63	///<
#define END_SYNC_POSITION	77	///<
#define END_SMPTE_POSITION	80	///<

static volatile uint32_t irq_us_previous = 0;
static volatile uint32_t irq_us_current = 0;

static volatile uint32_t bit_time = 0;
static volatile uint32_t total_bits = 0;
static volatile bool ones_bit_count = false;
static volatile uint32_t current_bit = 0;
static volatile uint32_t sync_count = 0;
static volatile bool timecode_sync = false;
static volatile bool timecode_valid = false;

static volatile uint8_t timecode_bits[8] ALIGNED;
static volatile char timecode[12] ALIGNED;
static volatile bool is_drop_frame_flag_set = false;

static volatile bool timecode_available = false;

static volatile uint32_t ltc_updates_per_seconde= (uint32_t) 0;
static volatile uint32_t ltc_updates_previous = (uint32_t) 0;
static volatile uint32_t ltc_updates = (uint32_t) 0;

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();

	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;

	dmb();
	ltc_updates_per_seconde = ltc_updates - ltc_updates_previous + 1;
	ltc_updates_previous = ltc_updates;

	dmb();
}

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();
#ifdef DEBUG
	bcm2835_gpio_set(RPI_V2_GPIO_P1_11);
#endif
	irq_us_current = BCM2835_ST->CLO;

	BCM2835_GPIO->GPEDS0 = 1 << RPI_V2_GPIO_P1_13;
	dmb();

	bit_time = irq_us_current - irq_us_previous;

	if ((bit_time < ONE_TIME_MIN) || (bit_time > ZERO_TIME_MAX)) {
		// Interrupt outside specifications;
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
						timecode_bits[n - 1] |= 0x80;
					}
					timecode_bits[n] = timecode_bits[n] >> 1;
				}

				if (current_bit == 1) {
					timecode_bits[7] |= 0x80;
				}
			}

			total_bits++;
		}

		if (total_bits == END_SMPTE_POSITION) {

			total_bits = 0;
			ones_bit_count = false;

			if (timecode_sync) {
				timecode_sync = false;
				timecode_valid = true;
			}
		}

		if (timecode_valid) {
			dmb();
			ltc_updates++;

			timecode_valid = false;

			timecode[10] = (timecode_bits[0] & 0x0F) + '0';	// frames
			timecode[9] = (timecode_bits[1] & 0x03) + '0';	// 10's of frames
			timecode[7] = (timecode_bits[2] & 0x0F) + '0';	// seconds
			timecode[6] = (timecode_bits[3] & 0x07) + '0';	// 10's of seconds
			timecode[4] = (timecode_bits[4] & 0x0F) + '0';	// minutes
			timecode[3] = (timecode_bits[5] & 0x07) + '0';	// 10's of minutes
			timecode[1] = (timecode_bits[6] & 0x0F) + '0';	// hours
			timecode[0] = (timecode_bits[7] & 0x03) + '0';	// 10's of hours

			is_drop_frame_flag_set = (timecode_bits[1] & (1 << 2));

			dmb();
			timecode_available = true;
		}
	}

	irq_us_previous = irq_us_current;
#ifdef DEBUG
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_11);
#endif
	dmb();
}

void notmain(void) {
	hardware_init();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);
	printf("TimeCode LTC Reader");

	console_set_top_row(3);

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_13, BCM2835_GPIO_FSEL_INPT);
#ifdef DEBUG
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_11, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_11);
#endif

	// Rising Edge
	BCM2835_GPIO->GPREN0 = 1 << RPI_V2_GPIO_P1_13;
	// Falling Edge
	BCM2835_GPIO->GPFEN0 = 1 << RPI_V2_GPIO_P1_13;
	// Clear status bit
	BCM2835_GPIO->GPEDS0 = 1 << RPI_V2_GPIO_P1_13;

	BCM2835_IRQ->IRQ_ENABLE2 = BCM2835_GPIO0_IRQn;
	dmb();

	__enable_irq();

	BCM2835_ST->CS = BCM2835_ST_CS_M1;
	BCM2835_ST->C1 = BCM2835_ST->CLO + (uint32_t) 1000000;
	BCM2835_IRQ->FIQ_CONTROL = BCM2835_FIQ_ENABLE | 1; // TODO replace with #define

	__enable_fiq();

	timecode[2] = ':';
	timecode[5] = ':';
	timecode[8] = '.';
	timecode[11] = '\0';

	uint8_t type = 0;

	for (;;) {
		dmb();
		if (timecode_available) {
			timecode_available = false;

			console_set_cursor(2,5);
			console_puts((char *)timecode);

			if (is_drop_frame_flag_set) {
				type = 2;
			} else {
				if (ltc_updates_per_seconde == 24) {
					type = 0;
				} else if (ltc_updates_per_seconde == 25) {
					type = 1;
				} else if (ltc_updates_per_seconde == 30) {
					type = 3;
				}
			}

			if (prev_type != type) {
				console_set_cursor(2, 6);
				console_puts(types[type]);
				prev_type = type;
			}

		}
	}
}

