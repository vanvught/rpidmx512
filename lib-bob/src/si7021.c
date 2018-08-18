#if defined(HAVE_I2C)
/**
 * @file si7021.c
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

#include <stdint.h>
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

extern void udelay(uint32_t);

#if defined(__linux__)
 #define udelay bcm2835_delayMicroseconds
#endif

#include "i2c.h"
#include "si7021.h"
#include "device_info.h"

#define SI7021_TEMP		0xF3
#define	SI7021_HUMID	0xF5

static uint8_t get_id(const device_info_t *device_info) {
	/* Page 23 https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf */
	char buffer[6] = { 0xFC, 0xC9 };

	i2c_write_nb(buffer, 2);
	(void) i2c_read(buffer, 6);

	return buffer[0];
}

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

bool si7021_start(device_info_t *device_info) {
	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = SI7021_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

#ifndef NDEBUG
	printf("get_id_2nd(device_info)=%x\n", get_id(device_info));
#endif

	/* Page 24 https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf
	 *
	 * 0x00 or 0xFF engineering samples
	 * 0x0D=13=Si7013
	 * 0x14=20=Si7020
	 * 0x15=21=Si7021
	 */

	if (get_id(device_info) != 0x15) {
		return false;
	}

	return true;
}

static const uint16_t get_raw_value(uint8_t cmd) {
	char buffer[3];

	i2c_write(cmd);

	udelay(80 * 1000);	// datasheet says 50ms

	(void) i2c_read(buffer, 3);

	return (((uint16_t) buffer[0] << 8) | ((uint16_t) buffer[1])) & (uint16_t) 0xFFFC;
}

float si7021_get_temperature(const device_info_t *device_info) {
	uint16_t value;
	float temp;

	i2c_setup(device_info);

	value = get_raw_value(SI7021_TEMP);

	temp = (float) value / 65536.0;

	return -46.85 + (175.72 * temp);
}

float si7021_get_humidity(const device_info_t *device_info) {
	uint16_t value;
	float humid;

	i2c_setup(device_info);

	value = get_raw_value(SI7021_HUMID);

	humid = (float) value / 65536.0;

	return -6.0 + (125.0 * humid);
}
#endif
