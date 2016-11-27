/**
 * @file rdm_sensor.h
 *
 * 10.7 Sensor Parameter Messages
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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


#ifndef RDM_SENSOR_H_
#define RDM_SENSOR_H_

#include <stdint.h>

struct _rdm_sensor_defintion {
	uint8_t sensor;
	uint8_t type;
	uint8_t unit;
	uint8_t prefix;
	int16_t range_min;
	int16_t range_max;
	int16_t normal_min;
	int16_t normal_max;
	uint8_t recorded_supported;
	uint8_t description[32];
	uint8_t _len;
};

struct _rdm_sensor_value {
	uint8_t sensor_requested;
	int16_t present;
	int16_t lowest_detected;
	int16_t highest_detected;
	int16_t recorded;
};

#define RDM_SENSOR_TEMPERATURE_RANGE_MIN	-32768		///<
#define RDM_SENSOR_TEMPERATURE_RANGE_MAX	+32767		///<
#define RDM_SENSOR_TEMPERATURE_NORMAL_MIN	-32768		///<
#define RDM_SENSOR_TEMPERATURE_NORMAL_MAX	+32767		///<

#define RDM_SENSOR_TEMPERATURE_ABS_ZERO		-273		///<

#define RDM_SENSOR_RECORDED_SUPPORTED		(1 << 0)	///<
#define RDM_SENSOR_LOW_HIGH_DETECT			(1 << 1)	///<

extern const uint8_t rdm_sensors_get_count(void);
extern void rdm_sensors_init(void);
extern /*@shared@*/const struct _rdm_sensor_defintion /*@null@*/*rdm_sensors_get_defintion(uint8_t);
extern /*@shared@*/struct _rdm_sensor_value /*@null@*/*rdm_sensors_get_value(uint8_t);
extern void rdm_sensors_set_value(uint8_t);
extern void rmd_sensors_record(uint8_t);

#endif /* RDM_SENSOR_H_ */
