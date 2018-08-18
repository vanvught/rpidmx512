#if defined(HAVE_I2C)
/**
 * @file ina219.c
 *
 */
/* This code is inspired by:
 *
 * https://github.com/jarzebski/Arduino-INA219
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
#include "ina219.h"
#include "device_info.h"

#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))

struct _ina219_info {
	float current_lsb;
	float power_lsb;
	float v_shunt_max;
	float v_bus_max;
	float r_shunt;
} static ina219_info[0x10] __attribute__((aligned(4)));

#define INA219_REG_CONFIG			0x00	///<
#define INA219_REG_SHUNTVOLTAGE		0x01	///<
#define INA219_REG_BUSVOLTAGE		0x02	///<
#define INA219_REG_POWER			0x03	///<
#define INA219_REG_CURRENT			0x04	///<
#define INA219_REG_CALIBRATION		0x05	///<

#define INA219_CONFIG_RESET					0x8000	///< Reset Bit

#define INA219_CONFIG_BVOLTAGERANGE_MASK	0x2000	///< Bus Voltage Range Mask
//#define INA219_CONFIG_BVOLTAGERANGE_SHIFT	13		///< Bus Voltage Range Shift

#define INA219_CONFIG_GAIN_MASK				0x1800	///< Gain Mask
//#define INA219_CONFIG_GAIN_SHIFT			11		///< Gain Shift

#define INA219_CONFIG_BADCRES_MASK			0x0780	///< Bus ADC Resolution Mask
//#define INA219_CONFIG_BADCRES_SHIFT			7		///< Bus ADC Resolution Shift

#define INA219_CONFIG_SADCRES_MASK			0x0078	///< Shunt ADC Resolution and Averaging Mask
//#define INA219_CONFIG_SADCRES_SHIFT			3		///< Shunt ADC Resolution and Averaging Shift

#define INA219_CONFIG_MODE_MASK				0x0007	///< Operating Mode Mask
//#define INA219_CONFIG_MODE_SHIFT			0		///< Operating Mode Shift

#define INA219_READ_REG_DELAY_US	800		///<

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

const bool ina219_start(device_info_t *device_info) {
	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = INA219_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	ina219_configure(device_info, INA219_RANGE_32V, INA219_GAIN_320MV, INA219_BUS_RES_12BIT, INA219_SHUNT_RES_12BIT_1S , INA219_MODE_SHUNT_BUS_CONT);

	ina219_calibrate(device_info, 0.1, 2);

	return true;
}

void ina219_configure(const device_info_t *device_info, ina219_range_t range, ina219_gain_t gain, ina219_bus_res_t bus_res, ina219_shunt_res_t shunt_res, ina219_mode_t mode) {
	const uint8_t i = device_info->slave_address & 0x0F;
	const uint16_t config = range | gain| bus_res | shunt_res | mode;

	switch (range) {
	case INA219_RANGE_32V:
		ina219_info[i].v_bus_max = (float) 32.0;
		break;
	case INA219_RANGE_16V:
		ina219_info[i].v_bus_max = (float) 16.0;
		break;
	}

    switch(gain)
    {
        case INA219_GAIN_320MV:
        	ina219_info[i].v_shunt_max = (float) 0.32;
            break;
        case INA219_GAIN_160MV:
        	ina219_info[i].v_shunt_max = (float) 0.16;
            break;
        case INA219_GAIN_80MV:
        	ina219_info[i].v_shunt_max = (float) 0.08;
            break;
        case INA219_GAIN_40MV:
        	ina219_info[i].v_shunt_max = (float) 0.04;
            break;
    }

    i2c_write_reg_uint16(INA219_REG_CONFIG, config);
}

void ina219_calibrate(const device_info_t *device_info, float r_shunt_value, float i_max_expected) {
	const uint8_t i = device_info->slave_address & 0x0F;
	const float minimum_lsb = i_max_expected / 32767;

	uint16_t calibration_value;

	ina219_info[i].r_shunt = r_shunt_value;

	ina219_info[i].current_lsb = (float) ((uint16_t) (minimum_lsb * 100000000));
	ina219_info[i].current_lsb /= 100000000;
	ina219_info[i].current_lsb /= 0.0001;
	ina219_info[i].current_lsb = (float) CEILING_POS(ina219_info[i].current_lsb);
	ina219_info[i].current_lsb *= 0.0001;

	ina219_info[i].power_lsb = ina219_info[i].current_lsb * 20;

	calibration_value = (uint16_t) ((0.04096) / (ina219_info[i].current_lsb * ina219_info[i].r_shunt));

	i2c_write_reg_uint16(INA219_REG_CALIBRATION, calibration_value);
}

const int16_t ina219_get_shunt_voltage_raw(const device_info_t *device_info) {
	int16_t voltage;

	i2c_setup(device_info);

	voltage = (int16_t) i2c_read_reg_uint16_delayus(INA219_REG_SHUNTVOLTAGE, (uint32_t) INA219_READ_REG_DELAY_US);

	return voltage;
}

const float ina219_get_shunt_voltage(const device_info_t *device_info) {
	const int16_t value = ina219_get_shunt_voltage_raw(device_info);

	return ((float) value / (float) 100000);
}

const int16_t ina219_get_bus_voltage_raw(const device_info_t *device_info) {
	uint16_t voltage;

	i2c_setup(device_info);

	voltage = i2c_read_reg_uint16_delayus(INA219_REG_BUSVOLTAGE, (uint32_t) INA219_READ_REG_DELAY_US);
	voltage >>= 3;

	return ((int16_t) voltage * (int16_t) 4);
}

const float ina219_get_bus_voltage(const device_info_t *device_info) {
	int16_t value;

	i2c_setup(device_info);

	value = ina219_get_bus_voltage_raw(device_info);

	return ((float) value * (float) 0.001);
}

const ina219_range_t ina219_get_range(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(INA219_REG_CONFIG);
	value &= INA219_CONFIG_BVOLTAGERANGE_MASK;

	return (ina219_range_t) value;
}

const ina219_gain_t ina219_get_gain(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(INA219_REG_CONFIG);
	value &= INA219_CONFIG_GAIN_MASK;

	return (ina219_gain_t) value;
}

const ina219_bus_res_t ina219_get_bus_res(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(INA219_REG_CONFIG);
	value &= INA219_CONFIG_BADCRES_MASK;

	return (ina219_bus_res_t) value;
}

const ina219_shunt_res_t ina219_get_shunt_res(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(INA219_REG_CONFIG);
	value &= INA219_CONFIG_SADCRES_MASK;

	return (ina219_shunt_res_t) value;
}

const ina219_mode_t ina219_get_mode(const device_info_t *device_info) {
	uint16_t value;

	i2c_setup(device_info);

	value = i2c_read_reg_uint16(INA219_REG_CONFIG);
	value &= INA219_CONFIG_MODE_MASK;

	return (ina219_mode_t) value;
}

const float ina219_get_max_possible_current(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	return (ina219_info[i].v_shunt_max / ina219_info[i].r_shunt);
}

const float ina219_get_max_current(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	const float max_current = ina219_info[i].current_lsb * 32767;
	const float max_possible = ina219_get_max_possible_current(device_info);

	if (max_current > max_possible) {
		return max_possible;
	} else {
		return max_current;
	}
}

const float ina219_get_max_shunt_voltage(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	const float max_voltage = ina219_get_max_current(device_info) * ina219_info[i].r_shunt;

	if (max_voltage >= ina219_info[i].v_shunt_max) {
		return ina219_info[i].v_shunt_max;
	} else {
		return max_voltage;
	}

}

const float ina219_get_max_power(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	return (ina219_get_max_current(device_info) * ina219_info[i].v_bus_max);
}

const float ina219_get_shunt_current(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	i2c_setup(device_info);

	return ((float)((int16_t)i2c_read_reg_uint16_delayus(INA219_REG_CURRENT, (uint32_t) INA219_READ_REG_DELAY_US)) * ina219_info[i].current_lsb);
}

const float ina219_get_bus_power(const device_info_t *device_info) {
	const uint8_t i = device_info->slave_address & 0x0F;

	i2c_setup(device_info);

	return ((float)((int16_t)i2c_read_reg_uint16_delayus(INA219_REG_POWER, (uint32_t) INA219_READ_REG_DELAY_US)) * ina219_info[i].power_lsb);
}

const uint16_t ina219_get_calibration(const device_info_t *device_info) {
	i2c_setup(device_info);

	return i2c_read_reg_uint16(INA219_REG_CALIBRATION);
}

void ina219_get_lsb(const device_info_t *device_info, float *current_lsb, float *power_lsb) {
	const uint8_t i = device_info->slave_address & 0x0F;

	*current_lsb = ina219_info[i].current_lsb;
	*power_lsb = ina219_info[i].power_lsb;
}
#endif
