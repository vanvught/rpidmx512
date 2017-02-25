
/**
 * @file pcf8591_i2c.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835_i2c.h"

#include "pcf8591.h"
#include "pcf8591_i2c.h"

#include "device_info.h"

/**
 *
 * @param device_info
 */
static void i2c_setup(const device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

/**
 *
 * @param device_info
 */
void pcf8591_i2c_start(device_info_t *device_info) {
	bcm2835_i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = PCF8591_I2C_DEFAULT_SLAVE_ADDRESS;
	}
}

/**
 *
 * @param
 * @param data
 */
void pcf8591_i2c_dac_write(device_info_t *device_info, const uint8_t data) {
	char cmd[2] = { (char) PCF8591_DAC_ENABLE, (char) 0x00 };

	cmd[1] = (char) data;

	i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));

}

/**
 *
 * @param device_info
 * @param channel
 * @return
 */
const uint8_t pcf8591_i2c_adc_read(device_info_t *device_info, const uint8_t channel) {
	char data = (uint8_t) channel;

	i2c_setup(device_info);
	bcm2835_i2c_write(&data, 1);
	bcm2835_i2c_read(&data, 1);
	bcm2835_i2c_read(&data, 1);

	return (uint8_t) data;
}
