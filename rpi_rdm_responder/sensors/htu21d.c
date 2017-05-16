/**
 * @file htu21d.c
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

#include "htu21d.h"

#include "dmx_devices.h"
#include "device_info.h"

#include "rdm_sensor.h"
#include "rdm_sensors.h"
#include "tables.h"

#include "rdm_e120.h"

static device_info_t device_info;

static const struct _rdm_sensor_defintion rdm_sensor_htu21d_t ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_TEMPERATURE,
		(uint8_t) E120_UNITS_CENTIGRADE,
		(uint8_t) E120_PREFIX_NONE,
		(int16_t) -40,
		(int16_t) 125,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Ambient Temperature", /* description length */(uint8_t) 19
};

static const struct _rdm_sensor_defintion rdm_sensor_htu21d_h ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_HUMIDITY,
		(uint8_t) E120_UNITS_NONE,
		(uint8_t) E120_PREFIX_NONE,
		(int16_t) 0,
		(int16_t) 100,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Relative Humidity", /* description length */(uint8_t) 17
};

/**
 *
 * @return
 */
static const int32_t htu21d_t(void) {
	return (int32_t) htu21d_get_temperature(&device_info);
}

/**
 *
 * @return
 */
static const int32_t htu21d_h(void) {
	return (int32_t) htu21d_get_humidity(&device_info);
}

/**
 *
 * @param f
 * @param address
 */
static void htu21d(/*@unused@*/struct _dmx_device_info *f, const uint8_t *address) {
	memset(&device_info, 0, sizeof(struct _device_info));
	device_info.slave_address = *address;

	if (htu21d_start(&device_info)) {
		(void) rdm_sensors_add(&rdm_sensor_htu21d_t, htu21d_t);
		(void) rdm_sensors_add(&rdm_sensor_htu21d_h, htu21d_h);
	}
}

INITIALIZER(sensors, htu21d)
