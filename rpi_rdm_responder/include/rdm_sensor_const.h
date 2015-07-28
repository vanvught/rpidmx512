/**
 * @file rdm_sensor_const.c
 *
 * @brief These definitions are private for the RDM Responder
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef RDM_SENSOR_CONST_H_
#define RDM_SENSOR_CONST_H_

#include <stdint.h>

#include "rdm_sensor.h"
#include "rdm_e120.h"
#include "hardware.h"

static const struct _rdm_sensor_defintion rdm_sensor_defintions[] __attribute__((aligned(4))) = { {
		(uint8_t) 0,
		(uint8_t) E120_SENS_TEMPERATURE,
		(uint8_t) E120_UNITS_CENTIGRADE,
		(uint8_t) E120_PREFIX_NONE,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) RDM_SENSOR_TEMPERATURE_RANGE_MAX,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) 85,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"CPU", /* description length */3 }
		};

#define RDM_SENSORS_COUNT	(sizeof(rdm_sensor_defintions) / sizeof(rdm_sensor_defintions[0]))

struct _rdm_sensor_func {
	const int32_t (*f)(void);
} static const rdm_sensor_funcs[RDM_SENSORS_COUNT] __attribute__((aligned(4))) = {{ hardware_get_core_temperature }};

static struct _rdm_sensor_value rdm_sensor_values[RDM_SENSORS_COUNT] __attribute__((aligned(4)));

#endif /* INCLUDE_RDM_SENSOR_CONST_H_ */
