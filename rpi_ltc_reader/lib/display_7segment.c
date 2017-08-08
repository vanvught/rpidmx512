/**
 * @file display_7segment.c
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

#include "d8x7segment.h"

#include "max7219.h"
#include "max7219_spi.h"

#include "device_info.h"

static device_info_t device_info;

/**
 *
 */
void display_7segment_init(const uint8_t intensity) {
	device_info.chip_select = 2;
	device_info.speed_hz = 0;

	d8x7segment_init(&device_info, intensity);
	d8x7segment_cls(&device_info);

	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT6, (uint8_t) 0x80);
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT4, (uint8_t) 0x80);
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT2, (uint8_t) 0x80);
}

/**
 *
 * @param timecode
 */
void display_7segment(const char *timecode) {
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT7, (uint8_t) (timecode[0] - '0'));
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT6, (uint8_t) (timecode[1] - '0') | 0x80);
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT5, (uint8_t) (timecode[3] - '0'));
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT4, (uint8_t) (timecode[4] - '0') | 0x80);
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT3, (uint8_t) (timecode[6] - '0'));
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT2, (uint8_t) (timecode[7] - '0') | 0x80);
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT1, (uint8_t) (timecode[9] - '0'));
	max7219_spi_write_reg(&device_info, MAX7219_REG_DIGIT0, (uint8_t) (timecode[10] - '0'));
}
