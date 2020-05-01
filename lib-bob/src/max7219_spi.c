/**
 * @file max7219_spi.c
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "bob.h"

#include "max7219.h"
#include "max7219_spi.h"

void max7219_spi_start(device_info_t *device_info) {

	if (device_info->speed_hz == 0) {
		device_info->speed_hz = MAX7219_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > MAX7219_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = MAX7219_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		FUNC_PREFIX(spi_begin());;
	}

}

void max7219_spi_write_reg(const device_info_t *device_info, uint32_t reg, uint32_t data) {
	const uint16_t spi_data = ((uint16_t) reg << 8) | (uint16_t) data;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_write(spi_data);
	} else {
		FUNC_PREFIX(spi_set_speed_hz(device_info->speed_hz));
		FUNC_PREFIX(spi_write(spi_data));
	}
}
