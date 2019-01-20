/**
 * @file mcp23017.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "mcp23017.h"
#include "mcp23x17.h"

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);
	i2c_set_baudrate(I2C_NORMAL_SPEED);
}

bool mcp23017_start(device_info_t *device_info) {
	if (!i2c_begin()) {
		return false;
	}

	if (device_info->slave_address == 0) {
		device_info->slave_address = MCP23017_DEFAULT_SLAVE_ADDRESS;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	return true;
}

uint16_t mcp23017_reg_read(const device_info_t *device_info, uint8_t reg) {
	i2c_setup(device_info);

	char buffer[2];

	buffer[0] = (char) reg;

	i2c_write_nb(buffer, 1);

	i2c_read((char *) buffer, (uint32_t) 2);

	return (uint16_t) ((uint16_t) buffer[1] << 8 | (uint16_t) buffer[0]);
}

void mcp23017_reg_write(const device_info_t *device_info, uint8_t reg, uint16_t data) {
	i2c_setup(device_info);

	char buffer[3];

	buffer[0] = (char) reg;
	buffer[1] = (char) (data & 0xFF);
	buffer[2] = (char) (data >> 8);

	i2c_write_nb(buffer, 3);
}

void mcp23017_gpio_fsel(const device_info_t *device_info, mcp23s17Pin pin, mcp23017FunctionSelect mode) {
	uint16_t data = mcp23017_reg_read(device_info, MCP23X17_IODIRA);

	if (mode == MCP23017_FSEL_OUTP) {
		data &= (~pin);
	} else {
		data |= pin;
	}

	mcp23017_reg_write(device_info, MCP23X17_IODIRA, data);
}

void mcp23017_gpio_set(const device_info_t *device_info, mcp23s17Pin pin) {
	uint16_t data = mcp23017_reg_read(device_info, MCP23X17_OLATA);

	data |= pin;

	mcp23017_reg_write(device_info, MCP23X17_GPIOA, data);
}

void mcp23017_gpio_clr(const device_info_t *device_info, mcp23s17Pin pin) {
	uint16_t data = mcp23017_reg_read(device_info, MCP23X17_OLATA);

	data &= (~pin);

	mcp23017_reg_write(device_info, MCP23X17_GPIOA, data);
}

uint8_t mcp23017_gpio_lev(const device_info_t *device_info, mcp23s17Pin pin) {
	const uint16_t data = mcp23017_reg_read(device_info, MCP23X17_GPIOA);

	if ((data & pin) == pin) {
		return 1;
	} else {
		return 0;
	}
}

void mcp23017_gpio_set_pud(const device_info_t *device_info, mcp23s17Pin pin, mcp23017PUDControl pud) {
	uint16_t data = mcp23017_reg_read(device_info, MCP23X17_GPPUA);

	if (pud == MCP23017_GPIO_PUD_OFF) {
		data &= (~pin);
	} else {
		data |= pin;
	}

	mcp23017_reg_write(device_info, MCP23X17_GPPUA, data);
}
