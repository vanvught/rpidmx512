#if defined(HAVE_I2C)
/**
 * @file mcp9808.c
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

#include "i2c.h"
#include "mcp9808.h"
#include "device_info.h"

#define MCP9808_REG_UPPER_TEMP		0x02	///<
#define MCP9808_REG_LOWER_TEMP		0x03	///<
#define MCP9808_REG_CRIT_TEMP		0x04	///<
#define MCP9808_REG_AMBIENT_TEMP	0x05	///<
#define MCP9808_REG_MANUF_ID		0x06	///<
#define MCP9808_REG_DEVICE_ID		0x07	///<

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

bool mcp9808_start(device_info_t *device_info) {
	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = MCP9808_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	if (i2c_read_reg_uint16(MCP9808_REG_MANUF_ID) != (uint16_t) 0x0054) {
		return false;
	}

	if (i2c_read_reg_uint16(MCP9808_REG_DEVICE_ID) != (uint16_t) 0x0400) {
		return false;
	}

	return true;
}

const float mcp9808_get_temperature(const device_info_t *device_info) {
	uint16_t val;
	float temp;

	i2c_setup(device_info);

	val = i2c_read_reg_uint16(MCP9808_REG_AMBIENT_TEMP);

	temp = (float) (val & (uint16_t) 0x0FFF);

	temp /= 16.0;

	if ((val & (uint16_t) 0x1000) == (uint16_t) 0x1000) {
		temp -= (float) 256;
	}

	return temp;
}
#endif
