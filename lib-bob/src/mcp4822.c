/**
 * @file mcp4822.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

/*
 * MCP4822: Dual 12-Bit Voltage Output DAC
 */

#include <stdint.h>
#include <stdbool.h>

#include "bob.h"

#include "mcp4822.h"
#include "mcp48x2.h"

#define MCP4822_DATA(x)	((uint16_t)((uint16_t)(x) & 0x0FFF))

bool mcp4822_start(device_info_t *device_info) {

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) MCP48X2_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) MCP48X2_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) MCP48X2_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	return true;
}

void mcp4822_write_a(const device_info_t *device_info, uint16_t data) {
	data = MCP4822_DATA(data) | 0x3000 | MCP48X2_WRITE_DAC_A;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_write(data);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_write(data);
	}
}

void mcp4822_write_b(const device_info_t *device_info, uint16_t data) {
	data = MCP4822_DATA(data) | 0x3000 | MCP48X2_WRITE_DAC_B;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_write(data);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_write(data);
	}

}

void mcp4822_write_ab(const device_info_t *device_info, uint16_t data_a, uint16_t data_b) {
	data_a = MCP4822_DATA(data_a) | 0x3000 | MCP48X2_WRITE_DAC_A;
	data_b = MCP4822_DATA(data_b) | 0x3000 | MCP48X2_WRITE_DAC_B;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_write(data_a);
		bcm2835_aux_spi_write(data_b);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_write(data_a);
		bcm2835_spi_write(data_b);
	}

}
