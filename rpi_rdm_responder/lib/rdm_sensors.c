/**
 * @file rdm_sensors.c
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "rdm_sensor.h"
#include "rdm_e120.h"
#include "hardware.h"
#include "util.h"

#include "tables.h"
#include "read_config_file.h"
#include "sscan.h"

TABLE(initializer_t, sensors)

static struct _rdm_sensor_defintion rdm_sensor_defintions[0xFF] ALIGNED = {
		{
		(uint8_t) 0,
		(uint8_t) E120_SENS_TEMPERATURE,
		(uint8_t) E120_UNITS_CENTIGRADE,
		(uint8_t) E120_PREFIX_NONE,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) 85,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"CPU", /* description length */(uint8_t) 3
		}
};

struct _rdm_sensor_func {
	const int32_t (*f)(void);
} static rdm_sensor_funcs[0xFF] ALIGNED = {{ hardware_get_core_temperature }};

static struct _rdm_sensor_value rdm_sensor_values[0xFF] ALIGNED;

static uint8_t rdm_sensors_count = (uint8_t) 1;

static void add_connected_sensor(const char *line) {
	int rc;
	char sensor_name[65] ALIGNED;
	uint8_t len = sizeof(sensor_name) - 1;
	uint8_t address;
	uint8_t channel;
	int i;

	memset(sensor_name, 0, sizeof(sensor_name));

	rc = sscan_i2c(line, sensor_name, &len, &address, &channel);

	if ((rc != 0) && (len != (uint8_t) 0)) {
		for (i = 0; i < TABLE_LENGTH(sensors); i++) {
			if (strcmp(sensors_table[i].name, sensor_name) == 0) {
				sensors_table[i].f(NULL, &address);
			}
		}
	}
}

/**
 *
 * @param rdm_sensor_defintion
 * @param f
 * @return
 */
const bool rdm_sensors_add(const struct _rdm_sensor_defintion *rdm_sensor_defintion, const void *f) {
	if (rdm_sensors_count == 0xFE) {
		return false;
	}

	if ((rdm_sensor_defintion == NULL) || (f == NULL)) {
		return false;
	}

	memcpy(&rdm_sensor_defintions[rdm_sensors_count], rdm_sensor_defintion, sizeof(struct _rdm_sensor_defintion));
	rdm_sensor_funcs[rdm_sensors_count].f = f;
	rdm_sensor_defintions[rdm_sensors_count].sensor = rdm_sensors_count;

	rdm_sensors_count++;

	return true;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_sensors_get_count(void) {
	return  rdm_sensors_count;
}

/**
 *
 */
void rdm_sensors_set_value_all(void) {
	uint8_t i;
	const uint8_t count = rdm_sensors_count;

	for (i = 0 ; i < count; i++) {
		int16_t value = (int16_t)rdm_sensor_funcs[i].f();

		rdm_sensor_values[i].recorded = value;
		rdm_sensor_values[i].lowest_detected = value;
		rdm_sensor_values[i].highest_detected = value;
		rdm_sensor_values[i].sensor_requested = i;
	}
}

/**
 * @ingroup rdm
 *
 */
void rdm_sensors_record_all(void) {
	uint8_t i;
	const uint8_t count = rdm_sensors_count;

	for (i = 0 ; i < count; i++) {
		int16_t value = (int16_t)rdm_sensor_funcs[i].f();

		rdm_sensor_values[i].recorded = value;
		rdm_sensor_values[i].lowest_detected = MIN(rdm_sensor_values[i].lowest_detected, value);
		rdm_sensor_values[i].highest_detected = MAX(rdm_sensor_values[i].highest_detected, value);
		rdm_sensor_values[i].sensor_requested = i;
	}
}

/**
 * @ingroup rdm
 *
 * @param sensor
 * @return
 */
const struct _rdm_sensor_defintion *rdm_sensors_get_defintion(const uint8_t sensor) {
	return &rdm_sensor_defintions[sensor];
}

/**
 * @ingroup rdm
 *
 * @param sensor
 * @return
 */
struct _rdm_sensor_value *rdm_sensors_get_value(const uint8_t sensor) {
	int16_t value;

	value = (int16_t)rdm_sensor_funcs[sensor].f();

	rdm_sensor_values[sensor].present = value;
	rdm_sensor_values[sensor].lowest_detected = MIN(rdm_sensor_values[sensor].lowest_detected, value);
	rdm_sensor_values[sensor].highest_detected = MAX(rdm_sensor_values[sensor].highest_detected, value);
	rdm_sensor_values[sensor].sensor_requested = sensor;

	return &rdm_sensor_values[sensor];
}

/**
 * @ingroup rdm
 *
 * @param sensor
 */
void rdm_sensors_set_value(const uint8_t sensor) {
	int16_t value;

	if (sensor == (uint8_t) 0xFF) {
		rdm_sensors_set_value_all();
	} else {
		value = (int16_t)rdm_sensor_funcs[sensor].f();
		rdm_sensor_values[sensor].lowest_detected = value;
		rdm_sensor_values[sensor].highest_detected = value;
		rdm_sensor_values[sensor].recorded = value;
		rdm_sensor_values[sensor].present = value;
		rdm_sensor_values[sensor].sensor_requested = sensor;
	}
}

/**
 * @ingroup rdm
 *
 * @param sensor
 */
void rmd_sensors_record(const uint8_t sensor) {
	if (sensor == (uint8_t) 0xFF) {
		rdm_sensors_record_all();
	} else {
		int16_t value = (int16_t) rdm_sensor_funcs[sensor].f();
		rdm_sensor_values[sensor].recorded = value;
		rdm_sensor_values[sensor].lowest_detected = MIN(rdm_sensor_values[sensor].lowest_detected, value);
		rdm_sensor_values[sensor].highest_detected = MAX(rdm_sensor_values[sensor].highest_detected, value);
		rdm_sensor_values[sensor].sensor_requested = sensor;
	}
}

/**
 * @ingroup rdm
 *
 */
void rdm_sensors_init(void) {
	rdm_sensors_count = (uint8_t)1;
	read_config_file("sensors.txt", &add_connected_sensor);
	rdm_sensors_set_value_all();
}
