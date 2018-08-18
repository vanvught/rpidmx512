/**
 * @file d8x8matrix.c
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

#include "bob.h"

#include "max7219.h"
#include "max7219_spi.h"

#include "font_cp437.h"

#ifndef MIN
#  define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

static uint8_t spi_data[64] __attribute__((aligned(4)));

static uint8_t rotate(const uint8_t r, const uint8_t x) {
	uint8_t y;
	uint8_t set, b;

	b = 0;

	for (y = 0; y < 8; y++) {
		set = cp437_font[r][y] & (1 << x);
		b |= (set != 0) ? (1 << y) : 0;
	}

	return b;
}

static void write_all(const device_info_t *device_info, const uint8_t reg, const uint8_t data) {
	uint8_t i;

	if ((device_info->internal.count * 2) > sizeof(spi_data)) {
		return;
	}

	for (i = 0; i < (device_info->internal.count * 2); i = i + 2) {
		spi_data[i] = reg;
		spi_data[i+1] = data;
	}

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb((const char *) spi_data, device_info->internal.count * 2);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb((const char *) spi_data, device_info->internal.count * 2);
	}

}

void d8x8matrix_cls(const device_info_t *device_info) {
	write_all(device_info, MAX7219_REG_DIGIT0, 0);
	write_all(device_info, MAX7219_REG_DIGIT1, 0);
	write_all(device_info, MAX7219_REG_DIGIT2, 0);
	write_all(device_info, MAX7219_REG_DIGIT3, 0);
	write_all(device_info, MAX7219_REG_DIGIT4, 0);
	write_all(device_info, MAX7219_REG_DIGIT5, 0);
	write_all(device_info, MAX7219_REG_DIGIT6, 0);
	write_all(device_info, MAX7219_REG_DIGIT7, 0);
}

void d8x8matrix_write(const device_info_t *device_info, const char *buf, uint8_t nbyte) {
	char c;
	uint8_t i;
	int j, k;

	if (nbyte > device_info->internal.count) {
		nbyte = device_info->internal.count;
	}

	for (i = 1; i < 9; i++) {
		k = (int) nbyte;

		for (j = 0; j < ((int) device_info->internal.count * 2) - ((int) nbyte * 2); j = j + 2) {
			spi_data[j] = MAX7219_REG_NOOP;
			spi_data[j + 1] = 0;
		}

		while (--k >= 0) {
			c = buf[k];
			spi_data[j++] = i;
			spi_data[j++] = rotate((uint8_t) c, 8 - i);
		}

		if (device_info->chip_select == SPI_CS2) {
			bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
			bcm2835_aux_spi_writenb((const char *) spi_data, (uint32_t) j);
		} else {
			bcm2835_spi_setClockDivider(device_info->internal.clk_div);
			bcm2835_spi_chipSelect(device_info->chip_select);
			bcm2835_spi_writenb((const char *) spi_data, (uint32_t) j);
		}
	}
}

void d8x8matrix_init(const device_info_t *device_info, const uint8_t count, const uint8_t intensity) {
	device_info_t *p = (device_info_t *)device_info;

	p->internal.count = MIN(count, sizeof(spi_data) / 2);

	(void) max7219_spi_start((device_info_t *) device_info);

	write_all(device_info, MAX7219_REG_SHUTDOWN, MAX7219_SHUTDOWN_NORMAL_OP);
	write_all(device_info, MAX7219_REG_DISPLAY_TEST, 0);
	write_all(device_info, MAX7219_REG_DECODE_MODE, 0);
	write_all(device_info, MAX7219_REG_SCAN_LIMIT, 7);

	write_all(device_info, MAX7219_REG_INTENSITY, intensity & 0x0F);

	d8x8matrix_cls(device_info);
}
