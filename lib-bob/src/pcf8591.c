#if defined(HAVE_I2C)
/**
 * @file pcf8591.c
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
#include "pcf8591.h"
#include "device_info.h"

#define PCF8591_DAC_ENABLE			0x40
#define PCF8591_ADC_AUTO_INC_MASK	0x44

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);
	i2c_set_baudrate(I2C_NORMAL_SPEED);
}

bool pcf8591_start(device_info_t *device_info) {
	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = PCF8591_DEFAULT_SLAVE_ADDRESS;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	return true;
}

void pcf8591_dac_write(const device_info_t *device_info, uint8_t data) {
	char cmd[2] = { (char) PCF8591_DAC_ENABLE, (char) 0x00 };

	cmd[1] = (char) data;

	i2c_setup(device_info);
	i2c_write_nb(cmd, sizeof(cmd) / sizeof(cmd[0]));

}

const uint8_t pcf8591_adc_read(const device_info_t *device_info, uint8_t channel) {
	uint8_t data = channel;

	i2c_setup(device_info);
	i2c_write(data);

	data = i2c_read_uint8();
	data = i2c_read_uint8();

	return data;
}
#endif
