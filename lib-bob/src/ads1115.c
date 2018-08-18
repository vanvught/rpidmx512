#if defined(HAVE_I2C)
/**
 * @file ads1115.c
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

#include "ads1x15.h"
#include "ads1115.h"

#include "device_info.h"

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

#if defined(BARE_METAL) && !defined(H3)
#include "bcm2835.h"
#define TIMEOUT_WAIT(stop_if_true, usec) 						\
do {															\
	const uint32_t micros_now = BCM2835_ST->CLO;				\
	do {														\
		if(stop_if_true)										\
			break;												\
	} while( BCM2835_ST->CLO - micros_now < (uint32_t)usec);	\
} while(0);
#else
 #define TIMEOUT_WAIT(stop_if_true, usec)
#endif

static void trigger_conversion(void) {
	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, ADS1x15_CONFIG_OS_SINGLE, ADS1x15_REG_CONFIG_OS_MASK);
}

static void poll_conversion(void) {
	TIMEOUT_WAIT((i2c_read_reg_uint16(ADS1x15_REG_CONFIG) == ADS1x15_CONFIG_OS_IDLE), 10000);
}

static void set_channel(const uint8_t channel) {
	uint16_t mux;

	switch (channel) {
		case ADS1115_CH0:
			mux = ADS1x15_REG_MUX_SINGLE_0;
			break;
		case ADS1115_CH1:
			mux = ADS1x15_REG_MUX_SINGLE_1;
			break;
		case ADS1115_CH2:
			mux = ADS1x15_REG_MUX_SINGLE_2;
			break;
		case ADS1115_CH3:
			mux = ADS1x15_REG_MUX_SINGLE_3;
			break;
		default:
			mux = ADS1x15_REG_MUX_SINGLE_0;
			break;
	}

	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG,
			(const uint16_t) (mux | ADS1x15_MODE_SINGLE_SHOT),
			ADS1x15_REG_CONFIG_MUX_MASK | ADS1x15_REG_CONFIG_MODE_MASK);

	trigger_conversion();
	poll_conversion();

	(void) i2c_read_reg_uint16(ADS1x15_REG_CONVERSION);

	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, ADS1x15_MODE_CONTINUOUS, ADS1x15_REG_CONFIG_MODE_MASK);
}

const bool ads1115_start(device_info_t *device_info) {
	uint16_t config;

	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = ADS1115_DEFAULT_SLAVE_ADDRESS;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	config = ADS1x15_REG_MUX_SINGLE_0			|
			 ADS1x15_PGA_6144					|
			 ADS1x15_MODE_SINGLE_SHOT			|
			 ADS1115_DATA_RATE_128				|
			 ADS1x15_COMP_POLARITY_ACTIVE_LO	|
			 ADS1x15_COMP_NON_LATCHING			|
			 ADS1x15_COMP_MODE_TRADITIONAL		|
			 ADS1x15_COMP_QUEUE_NONE;

	i2c_write_reg_uint16(ADS1x15_REG_CONFIG, config);

	set_channel(ADS1115_CH0);
	device_info->internal.adc_channel = ADS1115_CH0;

	return true;
}

const uint16_t ads1115_read(const device_info_t *device_info, const uint8_t channel) {
	device_info_t *d = (device_info_t *) device_info;

	if (channel > ADS1115_CH3) {
		return 0;
	}

	i2c_setup(device_info);

	if (channel != device_info->internal.adc_channel) {
		set_channel(channel);
		d->internal.adc_channel = channel;
	}

	return i2c_read_reg_uint16(ADS1x15_REG_CONVERSION);
}

const ads1115_data_rate_t ads1115_get_data_rate(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_DATA_RATE_MASK;

	return (ads1115_data_rate_t)value;
}

void ads1115_set_data_rate(const device_info_t *device_info, const ads1115_data_rate_t data_rate) {
	i2c_setup(device_info);
	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, (const uint16_t) data_rate, ADS1x15_REG_CONFIG_DATA_RATE_MASK);
}
#endif
