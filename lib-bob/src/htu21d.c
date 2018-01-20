/**
 * @file htu21d.c
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

#include "bcm2835.h"

#if defined(__linux__)
 #define udelay bcm2835_delayMicroseconds
#else
 #include "bcm2835_i2c.h"
#endif

#include "i2c.h"

#include "htu21d.h"

#include "device_info.h"

// TODO Add more
#define HTU21D_TEMP		0xF3
#define	HTU21D_HUMID	0xF5

/**
 *
 * @param device_info
 */
static void i2c_setup(const device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address);

	if (device_info->fast_mode) {
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
	} else {
		bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	}
}

/**
 *
 * @param device_info
 * @return
 */
const bool htu21d_start(device_info_t *device_info) {

	bcm2835_i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = HTU21D_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	return true;
}

/**
 *
 * @param cmd
 * @return
 */
static const uint16_t get_raw_value(const uint8_t cmd) {
	char buffer[3];

	buffer[0] = (char) cmd;
	bcm2835_i2c_write(buffer, 1);

	udelay(80 * 1000);	// datasheet says 50ms

	bcm2835_i2c_read(buffer, 3);

	return (((uint16_t) buffer[0] << 8) | ((uint16_t) buffer[1])) & (uint16_t) 0xFFFC;
}

/**
 *
 * @param device_info
 * @return
 */
const float htu21d_get_temperature(const device_info_t *device_info) {
	uint16_t value;
	float temp;

	i2c_setup(device_info);

	value = get_raw_value(HTU21D_TEMP);

	temp = (float) value / 65536.0;

	return -46.85 + (175.72 * temp);
}

/**
 *
 * @param device_info
 * @return
 */
const float htu21d_get_humidity(const device_info_t *device_info) {
	uint16_t value;
	float humid;

	i2c_setup(device_info);

	value = get_raw_value(HTU21D_HUMID);

	humid = (float) value / 65536.0;

	return -6.0 + (125.0 * humid);
}
