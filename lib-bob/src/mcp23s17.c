/**
 * @file mcp23s17.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "mcp23s17.h"
#include "mcp23x17.h"

bool mcp23s17_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = MCP23S17_DEFAULT_SLAVE_ADDRESS;
	} else {
		device_info->slave_address = device_info->slave_address & 0x03;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) MCP23S17_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) MCP23S17_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) MCP23S17_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
	}

	mcp23s17_reg_write_byte(device_info, MCP23X17_IOCON, MCP23X17_IOCON_HAEN);

	return true;
}

uint16_t mcp23s17_reg_read(const device_info_t *device_info, uint8_t reg) {
	char spiData[4];

	spiData[0] = (char) MCP23X17_CMD_READ | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_transfern(spiData, 4);
	} else {
		bcm2835_spi_set_speed_hz(device_info->speed_hz);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_transfern(spiData, 4);
	}

	return  ((uint16_t)spiData[2] | ((uint16_t)spiData[3] << 8));
}

void mcp23s17_reg_write(const device_info_t *device_info, uint8_t reg, uint16_t value) {
	char spiData[4];

	spiData[0] = (char) MCP23X17_CMD_WRITE | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;
	spiData[2] = (char) value;
	spiData[3] = (char) (value >> 8);

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(spiData, 4);
	} else {
		bcm2835_spi_set_speed_hz(device_info->speed_hz);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(spiData, 4);
	}
}

void mcp23s17_reg_write_byte(const device_info_t *device_info, uint8_t reg, uint8_t value) {
	char spiData[3];
	spiData[0] = (char) MCP23X17_CMD_WRITE | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;
	spiData[2] = (char) value;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(spiData, 3);
	} else {
		bcm2835_spi_set_speed_hz(device_info->speed_hz);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(spiData, 3);
	}
}

void mcp23s17_gpio_fsel(const device_info_t *device_info, uint16_t pin, uint8_t mode) {
	uint16_t data = mcp23s17_reg_read(device_info, MCP23X17_IODIRA);

	if (mode == MCP23S17_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}

	mcp23s17_reg_write(device_info, MCP23X17_IODIRA, data);
}

void mcp23s17_gpio_set(const device_info_t *device_info, uint16_t pin) {
	uint8_t data = mcp23s17_reg_read(device_info, MCP23X17_OLATA);
	data |= pin;
	mcp23s17_reg_write(device_info, MCP23X17_GPIOA, data);
}

void mcp23s17_gpio_clr(const device_info_t *device_info, uint16_t pin) {
	uint8_t data = mcp23s17_reg_read(device_info, MCP23X17_OLATA);
	data &= (~pin);
	mcp23s17_reg_write(device_info, MCP23X17_GPIOA, data);
}
