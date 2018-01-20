/**
 * @file ina219.h
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

#ifndef INA219_H_
#define INA219_H_

#include <stdbool.h>

#include "device_info.h"

#define INA219_I2C_DEFAULT_SLAVE_ADDRESS	0x40	///<

typedef enum ina219_range {
	INA219_RANGE_16V = 0x0000,	///< 0-16V Range
	INA219_RANGE_32V = 0x2000,	///< 0-32V Range
} ina219_range_t;

typedef enum ina219_gain {
	INA219_GAIN_40MV  = 0x0000,	///< Gain 1, 40mV Range
	INA219_GAIN_80MV  = 0x0800,	///< Gain 2, 80mV Range
	INA219_GAIN_160MV = 0x1000,	///< Gain 4, 160mV Range
	INA219_GAIN_320MV = 0x1800  ///< Gain 8, 320mV Range
} ina219_gain_t;

typedef enum ina219_bus_res {
	INA219_BUS_RES_9BIT  = 0x0080,	///< 9-bit bus res = 0..511
	INA219_BUS_RES_10BIT = 0x0100,	///< 10-bit bus res = 0..1023
	INA219_BUS_RES_11BIT = 0x0200,	///< 11-bit bus res = 0..2047
	INA219_BUS_RES_12BIT = 0x0400	///< 12-bit bus res = 0..4097
} ina219_bus_res_t;

typedef enum ina219_shunt_res {
	INA219_SHUNT_RES_9BIT_1S    = 0x0000,	///< 1 x 9-bit shunt sample
	INA219_SHUNT_RES_10BIT_1S   = 0x0008,	///< 1 x 10-bit shunt sample
	INA219_SHUNT_RES_11BIT_1S   = 0x0010,	///< 1 x 11-bit shunt sample
	INA219_SHUNT_RES_12BIT_1S   = 0x0018,	///< 1 x 12-bit shunt sample
	INA219_SHUNT_RES_12BIT_2S   = 0x0048,	///< 2 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_4S   = 0x0050,	///< 4 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_8S   = 0x0058,	///< 8 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_16S  = 0x0060,	///< 16 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_32S  = 0x0068,	///< 32 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_64S  = 0x0070,	///< 64 x 12-bit shunt samples averaged together
	INA219_SHUNT_RES_12BIT_128S = 0x0078	///< 128 x 12-bit shunt samples averaged together
} ina219_shunt_res_t;

typedef enum ina219_mode {
	INA219_MODE_POWER_DOWN     = 0x0000,	///<
	INA219_MODE_SHUNT_TRIG     = 0x0001,	///<
	INA219_MODE_BUS_TRIG       = 0x0002,	///<
	INA219_MODE_SHUNT_BUS_TRIG = 0x0003,	///<
	INA219_MODE_ADC_OFF        = 0x0004,	///<
	INA219_MODE_SHUNT_CONT     = 0x0005,	///<
	INA219_MODE_BUS_CONT       = 0x0006,	///<
	INA219_MODE_SHUNT_BUS_CONT = 0x0007		///<
} ina219_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const bool ina219_start(device_info_t *);

extern void ina219_configure(const device_info_t *, ina219_range_t, ina219_gain_t, ina219_bus_res_t, ina219_shunt_res_t, ina219_mode_t);
extern void ina219_calibrate(const device_info_t *, float, float);

extern const ina219_range_t ina219_get_range(const device_info_t *);
extern const ina219_gain_t ina219_get_gain(const device_info_t *);
extern const ina219_bus_res_t ina219_get_bus_res(const device_info_t *);
extern const ina219_shunt_res_t ina219_get_shunt_res(const device_info_t *);
extern const ina219_mode_t ina219_get_mode(const device_info_t *);

extern const float ina219_get_max_possible_current(const device_info_t *);
extern const float ina219_get_max_current(const device_info_t *);
extern const float ina219_get_max_shunt_voltage(const device_info_t *);
extern const float ina219_get_max_power(const device_info_t *);

extern const int16_t ina219_get_shunt_voltage_raw(const device_info_t *);
extern const float ina219_get_shunt_voltage(const device_info_t *);

extern const int16_t ina219_get_bus_voltage_raw(const device_info_t *);
extern const float ina219_get_bus_voltage(const device_info_t *);

extern const float ina219_get_shunt_current(const device_info_t *);
extern const float ina219_get_bus_power(const device_info_t *);

extern const uint16_t ina219_get_calibration(const device_info_t *);

extern void ina219_get_lsb(const device_info_t *, float *, float *);

#ifdef __cplusplus
}
#endif

#endif /* INA219_H_ */
