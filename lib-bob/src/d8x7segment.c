/**
 * @file d8x7segment.c
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
#include <stdbool.h>

#include "max7219.h"
#include "max7219_spi.h"

#include "device_info.h"

#include "debug.h"

void d8x7segment_init(const device_info_t *device_info, uint8_t intensity) {
	DEBUG1_ENTRY

	(void) max7219_spi_start((device_info_t *)device_info);

	max7219_spi_write_reg(device_info, MAX7219_REG_SHUTDOWN, MAX7219_SHUTDOWN_NORMAL_OP);
	max7219_spi_write_reg(device_info, MAX7219_REG_DISPLAY_TEST, 0);
	max7219_spi_write_reg(device_info, MAX7219_REG_DECODE_MODE, MAX7219_DECODE_MODE_CODEB);
	max7219_spi_write_reg(device_info, MAX7219_REG_SCAN_LIMIT, 7);

	max7219_spi_write_reg(device_info, MAX7219_REG_INTENSITY, intensity & 0x0F);

	DEBUG1_EXIT
}

void d8x7segment_cls(const device_info_t *device_info) {
	uint8_t i = 8;

	do {
		max7219_spi_write_reg(device_info, i, MAX7219_CHAR_BLANK);
	} while (--i > (uint8_t) 0);
}

void d8x7segment_int(const device_info_t *device_info, int32_t number) {
	bool is_negative = false;
	uint8_t max_digits = 8;
	uint8_t i = 1;

	d8x7segment_cls(device_info);

	if (number == 0) {
		max7219_spi_write_reg(device_info, MAX7219_REG_DIGIT0, 0);
		return;
	}

	if (number < (int32_t) 0) {
		is_negative = true;
		max_digits = 7;
		number *= -1;
	}

	do {
		max7219_spi_write_reg(device_info, i++, (uint8_t) (number % 10));
		number /= 10;
	} while ((number != (int32_t) 0) && (i <= (uint8_t) max_digits));

	if ((number != (int32_t) 0)) {
		max7219_spi_write_reg(device_info, (i - 1), MAX7219_CHAR_E);
	}

	if (is_negative) {
		max7219_spi_write_reg(device_info, i, MAX7219_CHAR_NEGATIVE);
	}
}

