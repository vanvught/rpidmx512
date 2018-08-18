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

#define MCP23S17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23S17_IODIRB			0x01	///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
#define MCP23S17_IPOLA			0x02	///< INPUT POLARITY (IPOLA) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
#define MCP23S17_IPOLB			0x03	///< INPUT POLARITY (IPOLB) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
#define MCP23S17_GPINTENA		0x04	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENA) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23S17_GPINTENB		0x05	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENB) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23S17_DEFVALA		0x06	///< DEFAULT COMPARE (DEFVALA) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
#define MCP23S17_DEFVALB		0x07	///< DEFAULT COMPARE (DEFVALB) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
#define MCP23S17_INTCONA		0x08	///< INTERRUPT CONTROL (INTCONA) REGISTER, 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
#define MCP23S17_INTCONB		0x09	///< INTERRUPT CONTROL (INTCONB) REGISTER. 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
#define MCP23S17_IOCON			0x0A	///< CONFIGURATION (IOCON) REGISTER
//								0x0B	///< CONFIGURATION (IOCON) REGISTER
#define MCP23S17_GPPUA			0x0C	///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23S17_GPPUB			0x0D	///< PULL-UP RESISTOR CONFIGURATION (GPPUB) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23S17_INTFA			0x0E	///< INTERRUPT FLAG (INTFA) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
#define MCP23S17_INTFB			0x0F	///< INTERRUPT FLAG (INTFB) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
#define MCP23S17_INTCAPA		0x10	///< INTERRUPT CAPTURE (INTCAPA) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23S17_INTCAPB		0x11	///< INTERRUPT CAPTURE (INTCAPB) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23S17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23S17_GPIOB			0x13	///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
#define MCP23S17_OLATA			0x14	///< OUTPUT LATCH REGISTER (OLATA), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value
#define MCP23S17_OLATB			0x15	///< OUTPUT LATCH REGISTER (OLATB), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value

#define MCP23S17_CMD_WRITE		0x40
#define MCP23S17_CMD_READ		0x41

#define MCP23S17_IOCON_HAEN		(uint8_t)(1 << 3)

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
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	mcp23s17_reg_write_byte(device_info, MCP23S17_IOCON, MCP23S17_IOCON_HAEN);

	return true;
}

uint16_t mcp23s17_reg_read(const device_info_t *device_info, uint8_t reg) {
	char spiData[4];

	spiData[0] = (char) MCP23S17_CMD_READ | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_transfern(spiData, 4);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_transfern(spiData, 4);
	}

	return  ((uint16_t)spiData[2] | ((uint16_t)spiData[3] << 8));
}

void mcp23s17_reg_write(const device_info_t *device_info, uint8_t reg, uint16_t value) {
	char spiData[4];

	spiData[0] = (char) MCP23S17_CMD_WRITE | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;
	spiData[2] = (char) value;
	spiData[3] = (char) (value >> 8);

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(spiData, 4);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(spiData, 4);
	}
}

void mcp23s17_reg_write_byte(const device_info_t *device_info, uint8_t reg, uint8_t value) {
	char spiData[3];
	spiData[0] = (char) MCP23S17_CMD_WRITE | (char) ((device_info->slave_address) << 1);
	spiData[1] = (char) reg;
	spiData[2] = (char) value;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(spiData, 3);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(spiData, 3);
	}
}

void mcp23s17_gpio_fsel(const device_info_t *device_info, uint16_t pin, uint8_t mode) {
	uint8_t data = mcp23s17_reg_read(device_info, MCP23S17_IODIRA);

	if (mode == MCP23S17_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}

	mcp23s17_reg_write(device_info, MCP23S17_IODIRA, data);
}

void mcp23s17_gpio_set(const device_info_t *device_info, uint16_t pin) {
	uint8_t data = mcp23s17_reg_read(device_info, MCP23S17_OLATA);
	data |= pin;
	mcp23s17_reg_write(device_info, MCP23S17_GPIOA, data);
}

void mcp23s17_gpio_clr(const device_info_t *device_info, uint16_t pin) {
	uint8_t data = mcp23s17_reg_read(device_info, MCP23S17_OLATA);
	data &= (~pin);
	mcp23s17_reg_write(device_info, MCP23S17_GPIOA, data);
}
