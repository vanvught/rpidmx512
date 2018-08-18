/**
 * @file mcp23s08.c
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

#include "mcp23s08.h"

#define MCP23S08_IODIR					0x00	///< I/O DIRECTION (IODIR) REGISTER
#define MCP23S08_IPOL					0x01	///< INPUT POLARITY (IPOL) REGISTER
#define MCP23S08_GPINTEN				0x02	///< INTERRUPT-ON-CHANGE CONTROL (GPINTEN) REGISTER
#define MCP23S08_DEFVAL					0x03	///< DEFAULT COMPARE (DEFVAL) REGISTER FOR INTERRUPT-ON-CHANGE
#define MCP23S08_INTCON					0x04	///< INTERRUPT CONTROL (INTCON) REGISTER
#define MCP23S08_IOCON					0x05	///< CONFIGURATION (IOCON) REGISTER
#define MCP23S08_GPPU					0x06	///< PULL-UP RESISTOR CONFIGURATION (GPPU) REGISTER
#define MCP23S08_INTF					0x07	///< INTERRUPT FLAG (INTF) REGISTER
#define MCP23S08_INTCAP					0x08	///< INTERRUPT CAPTURE (INTCAP) REGISTER
#define MCP23S08_GPIO					0x09	///< PORT (GPIO) REGISTER
#define MCP23S08_OLAT					0x0A	///< OUTPUT LATCH REGISTER (OLAT)

#define MCP23S08_CMD_WRITE				0x40
#define MCP23S08_CMD_READ				0x41

#define MCP23S08_IOCON_HAEN				(uint8_t)(1 << 3)

bool mcp23s08_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = MCP23S08_DEFAULT_SLAVE_ADDRESS;
	} else {
		device_info->slave_address = device_info->slave_address & 0x03;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) MCP23S08_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) MCP23S08_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) MCP23S08_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	mcp23s08_reg_write(device_info, MCP23S08_IOCON, MCP23S08_IOCON_HAEN);

	return true;
}

uint8_t mcp23s08_reg_read(const device_info_t *device_info, uint8_t reg) {
	char spiData[3];

	spiData[0] = (char) MCP23S08_CMD_READ | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_transfern(spiData, 3);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_transfern(spiData, 3);
	}

	return (uint8_t) spiData[2];
}

void mcp23s08_reg_write(const device_info_t *device_info, uint8_t reg, uint8_t value) {
	char spiData[3];

	spiData[0] = (char) MCP23S08_CMD_WRITE	| (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;
	spiData[2] = (char) value;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(spiData, 3);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_writenb(spiData, 3);
	}
}

void mcp23s08_gpio_fsel(const device_info_t *device_info, uint8_t pin, uint8_t mode) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_IODIR);

	if (mode == MCP23S08_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}

	mcp23s08_reg_write(device_info, MCP23S08_IODIR, data);
}

void mcp23s08_gpio_set(const device_info_t *device_info, uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data |= pin;
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}

void mcp23s08_gpio_clr(const device_info_t *device_info, uint8_t pin) {
	uint8_t data = mcp23s08_reg_read(device_info, MCP23S08_OLAT);
	data &= (~pin);
	mcp23s08_reg_write(device_info, MCP23S08_GPIO, data);
}
