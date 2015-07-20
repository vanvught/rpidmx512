/**
 * @file rdm_sensors.c
 *
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

#include <stdint.h>
#include <string.h>

#include "util.h"
#include "rdm_sensor_const.h"

/**
 *
 * @return
 */
const uint8_t rdm_sensors_get_count(void) {
	return  RDM_SENSORS_COUNT;
}

/**
 *
 */
void rdm_sensors_init(void) {
	uint8_t i;
	const uint8_t count = sizeof(rdm_sensor_funcs) / sizeof(rdm_sensor_funcs[0]);
	const uint8_t values = sizeof(rdm_sensor_values) / sizeof(rdm_sensor_values[0]);

	if (RDM_SENSORS_COUNT == 0) {
		return;
	}

	for (i = 0 ; (i < count) && (i < values); i++) {
		int16_t value = (int16_t)rdm_sensor_funcs[i].f();
		rdm_sensor_values[i].sensor_requested = i;
		rdm_sensor_values[i].lowest_detected = value;
		rdm_sensor_values[i].highest_detected = value;
		rdm_sensor_values[i].recorded = value;
	}
}

/**
 *
 * @param sensor
 * @return
 */
const struct _rdm_sensor_defintion *rdm_sensors_get_defintion(uint8_t sensor) {
	if (sensor > RDM_SENSORS_COUNT -1) {
		return NULL;
	}

	return &rdm_sensor_defintions[sensor];
}

/**
 *
 * @param sensor
 * @return
 */
struct _rdm_sensor_value *rdm_sensors_get_value(uint8_t sensor) {
	int16_t value;

	if (sensor > RDM_SENSORS_COUNT -1) {
		return NULL;
	}

	value = (int16_t)rdm_sensor_funcs[sensor].f();

	rdm_sensor_values[sensor].present = value;
	rdm_sensor_values[sensor].lowest_detected = MIN(rdm_sensor_values[sensor].lowest_detected, value);
	rdm_sensor_values[sensor].highest_detected = MAX(rdm_sensor_values[sensor].highest_detected, value);

	return &rdm_sensor_values[sensor];
}

/**
 *
 * @param sensor
 */
void rdm_sensors_set_value(uint8_t sensor) {
	int16_t value;

	if (sensor > RDM_SENSORS_COUNT -1) {
		return;
	}

	value = (int16_t)rdm_sensor_funcs[sensor].f();

	rdm_sensor_values[sensor].lowest_detected = value;
	rdm_sensor_values[sensor].highest_detected = value;
	rdm_sensor_values[sensor].recorded = value;
	rdm_sensor_values[sensor].present = value;
}

/**
 *
 * @param sensor
 */
void rmd_sensors_record(uint8_t sensor) {
	if (sensor > RDM_SENSORS_COUNT -1) {
		return ;
	}

	if (sensor == 0xFF) {
		rdm_sensors_init();
	} else {
		int16_t value = (int16_t)rdm_sensor_funcs[sensor].f();
		rdm_sensor_values[sensor].recorded = value;
		rdm_sensor_values[sensor].lowest_detected = MIN(rdm_sensor_values[sensor].lowest_detected, value);
		rdm_sensor_values[sensor].highest_detected = MAX(rdm_sensor_values[sensor].highest_detected, value);
	}
}
