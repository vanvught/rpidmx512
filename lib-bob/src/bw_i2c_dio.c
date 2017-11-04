/**
 * @file bw_i2c_dio.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835.h"
#include "bcm2835_i2c.h"

#include "i2c.h"

#include "bw.h"
#include "bw_dio.h"

#include "device_info.h"

#define BW_DIO_I2C_BYTE_WAIT_US		0

/**
 * @ingroup I2C-DIO
 *
 * @param device_info
 */
inline static void dio_i2c_setup(const device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

/**
 * @ingroup I2C-DIO
 *
 * @param device_info
 * @return
 */
const bool bw_i2c_dio_start(device_info_t *device_info) {
	bcm2835_i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;
	}

	dio_i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address >> 1)) {
		return false;
	}

	return true;
}

/**
 * @ingroup I2C-DIO
 *
 * @param device_info
 * @param mask
 */
void bw_i2c_dio_fsel_mask(const device_info_t *device_info, const uint8_t mask) {
	char cmd[2];

	cmd[0] = (char) BW_PORT_WRITE_IO_DIRECTION;
	cmd[1] = (char) mask;

	dio_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-DIO
 *
 * @param device_info
 * @param pins
 */
void bw_i2c_dio_output(const device_info_t *device_info, const uint8_t pins) {
	char cmd[2];

	cmd[0] = (char) BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[1] = (char) pins;

	dio_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}
