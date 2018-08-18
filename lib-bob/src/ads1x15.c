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

#include "i2c.h"

#include "ads1x15.h"

#include "device_info.h"

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

uint16_t ads1x15_get_op_status(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_OS_MASK;

	return value;
}

ads1x15_mux_t ads1x15_get_mux(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_MUX_MASK;

	return (ads1x15_mux_t)value;
}

void ads1x15_set_mux(const device_info_t *device_info, const ads1x15_mux_t mux) {
	i2c_setup(device_info);
	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, (const uint16_t) mux, ADS1x15_REG_CONFIG_MUX_MASK);
}

ads1x15_pga_t ads1x15_get_pga(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_PGA_MASK;

	return (ads1x15_pga_t)value;
}

void ads1x15_set_pga(const device_info_t *device_info, const ads1x15_pga_t pga) {
	i2c_setup(device_info);
	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, (const uint16_t) pga, ADS1x15_REG_CONFIG_PGA_MASK);
}

ads1x15_mode_t ads1x15_get_mode(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_MODE_MASK;

	return (ads1x15_mode_t)value;
}

void ads1x15_set_mode(const device_info_t *device_info, const ads1x15_mode_t mode) {
	i2c_setup(device_info);
	i2c_write_reg_uint16_mask(ADS1x15_REG_CONFIG, (const uint16_t) mode, ADS1x15_REG_CONFIG_MODE_MASK);
}

ads1x15_comp_mode_t ads1x15_get_comp_mode(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_COMP_MODE_MASK;

	return (ads1x15_comp_mode_t)value;
}

const ads1x15_comp_polarity_t ads1x15_get_comp_polarity(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_COMP_POLARITY_MASK;

	return (ads1x15_comp_polarity_t)value;
}

ads1x15_comp_latching_t ads1x15_get_comp_latching(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_COMP_LATCHING_MASK;

	return (ads1x15_comp_latching_t)value;
}

ads1x15_comp_queue_t ads1x15_get_comp_queue(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(ADS1x15_REG_CONFIG);
	value &= ADS1x15_REG_CONFIG_COMP_QUEUE_MASK;

	return (ads1x15_comp_queue_t)value;
}
#endif
