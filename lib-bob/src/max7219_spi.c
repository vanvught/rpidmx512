/**
 * @file max7219_spi.c
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

#include "bob.h"

#include "max7219.h"
#include "max7219_spi.h"

#include "debug.h"

void max7219_spi_start(device_info_t *device_info) {
	DEBUG_ENTRY

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) MAX7219_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) MAX7219_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) MAX7219_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	DEBUG_EXIT
}

void max7219_spi_write_reg(const device_info_t *device_info, uint8_t reg, uint8_t data) {
	const uint16_t spi_data = ((uint16_t) reg << 8) | (uint16_t) data;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_write(spi_data);
	} else {
		//bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		//bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_write(spi_data);
	}
}
