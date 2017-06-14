/**
 * @file ads1115.c
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

#include "ads1115.h"
#include "acs71x.h"

#include "dmx_devices.h"
#include "device_info.h"

#include "rdm_sensor.h"
#include "rdm_sensors.h"
#include "tables.h"

#include "rdm_e120.h"
#include "ads1115_params.h"

static struct _ads1115_ch_info *ads1115_ch_info;

static acs71x_info_t acs71x_info_adc_ch0;
static acs71x_info_t acs71x_info_adc_ch1;
static acs71x_info_t acs71x_info_adc_ch2;
static acs71x_info_t acs71x_info_adc_ch3;

static device_info_t device_info;

static struct _rdm_sensor_defintion rdm_sensor_acs71x_ch0 ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_CURRENT,
		(uint8_t) E120_UNITS_AMPERE_DC,
		(uint8_t) E120_PREFIX_MILLI,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Current", /* description length */(uint8_t) 7
};

static struct _rdm_sensor_defintion rdm_sensor_acs71x_ch1 ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_CURRENT,
		(uint8_t) E120_UNITS_AMPERE_DC,
		(uint8_t) E120_PREFIX_MILLI,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Current", /* description length */(uint8_t) 7
};

static struct _rdm_sensor_defintion rdm_sensor_acs71x_ch2 ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_CURRENT,
		(uint8_t) E120_UNITS_AMPERE_DC,
		(uint8_t) E120_PREFIX_MILLI,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Current", /* description length */(uint8_t) 7
};

static struct _rdm_sensor_defintion rdm_sensor_acs71x_ch3 ALIGNED = {
		(uint8_t) 0 /* dummy value */,
		(uint8_t) E120_SENS_CURRENT,
		(uint8_t) E120_UNITS_AMPERE_DC,
		(uint8_t) E120_PREFIX_MILLI,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(int16_t) RDM_SENSOR_RANGE_MIN,
		(int16_t) RDM_SENSOR_RANGE_MAX,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"Current", /* description length */(uint8_t) 7
};

/**
 *
 * @return
 */
static const int32_t ads1115_ch0(void) {
	return (int32_t) acs71x_get_current_dc(&acs71x_info_adc_ch0);
}

/**
 *
 * @return
 */
static const int32_t ads1115_ch1(void) {
	return (int32_t) acs71x_get_current_dc(&acs71x_info_adc_ch1);
}

/**
 *
 * @return
 */
static const int32_t ads1115_ch2(void) {
	return (int32_t) acs71x_get_current_dc(&acs71x_info_adc_ch2);
}

/**
 *
 * @return
 */
static const int32_t ads1115_ch3(void) {
	return (int32_t) acs71x_get_current_dc(&acs71x_info_adc_ch3);
}

/**
 *
 * @param f
 * @param address
 */
static void ads1115(/*@unused@*/struct _dmx_device_info *f, const uint8_t *address) {
	ads1115_params_init();

	ads1115_ch_info = (struct _ads1115_ch_info *)ads1115_params_get_ch_info();

	memset(&device_info, 0, sizeof(struct _device_info));
	device_info.slave_address = *address;
	device_info.fast_mode = true;

	if (ads1115_ch_info->ch0.is_connected) {
		acs71x_info_adc_ch0.type = ads1115_ch_info->ch0.type;
		acs71x_info_adc_ch0.adc = ACS71X_ADC_ADS1115;
		acs71x_info_adc_ch0.channel = ACS71X_ADC_CH0;
		acs71x_info_adc_ch0.calibrate = ads1115_ch_info->calibrate;
		acs71x_info_adc_ch0.device_info = &device_info;

		const uint8_t range = acs71x_get_range(&acs71x_info_adc_ch0);

		rdm_sensor_acs71x_ch0.range_min = (uint16_t) -range * 1000;
		rdm_sensor_acs71x_ch0.range_max = (uint16_t) range * 1000;

		if (acs71x_start(&acs71x_info_adc_ch0)) {
			(void) rdm_sensors_add(&rdm_sensor_acs71x_ch0, ads1115_ch0);
		}
	}

	if (ads1115_ch_info->ch1.is_connected) {
		acs71x_info_adc_ch1.type = ads1115_ch_info->ch1.type;
		acs71x_info_adc_ch1.adc = ACS71X_ADC_ADS1115;
		acs71x_info_adc_ch1.channel = ACS71X_ADC_CH1;
		acs71x_info_adc_ch1.calibrate = ads1115_ch_info->calibrate;
		acs71x_info_adc_ch1.device_info = &device_info;

		const uint8_t range = acs71x_get_range(&acs71x_info_adc_ch1);

		rdm_sensor_acs71x_ch1.range_min = (uint16_t) -range * 1000;
		rdm_sensor_acs71x_ch1.range_max = (uint16_t) range * 1000;

		if (acs71x_start(&acs71x_info_adc_ch1)) {
			(void) rdm_sensors_add(&rdm_sensor_acs71x_ch1, ads1115_ch1);
		}
	}

	if (ads1115_ch_info->ch2.is_connected) {
		acs71x_info_adc_ch2.type = ads1115_ch_info->ch2.type;
		acs71x_info_adc_ch2.adc = ACS71X_ADC_ADS1115;
		acs71x_info_adc_ch2.channel = ACS71X_ADC_CH2;
		acs71x_info_adc_ch2.calibrate = ads1115_ch_info->calibrate;
		acs71x_info_adc_ch2.device_info = &device_info;

		const uint8_t range = acs71x_get_range(&acs71x_info_adc_ch2);

		rdm_sensor_acs71x_ch2.range_min = (uint16_t) -range * 1000;
		rdm_sensor_acs71x_ch2.range_max = (uint16_t) range * 1000;

		if (acs71x_start(&acs71x_info_adc_ch2)) {
			(void) rdm_sensors_add(&rdm_sensor_acs71x_ch2, ads1115_ch2);
		}
	}

	if (ads1115_ch_info->ch3.is_connected) {
		acs71x_info_adc_ch3.type = ads1115_ch_info->ch3.type;
		acs71x_info_adc_ch3.adc = ACS71X_ADC_ADS1115;
		acs71x_info_adc_ch3.channel = ACS71X_ADC_CH3;
		acs71x_info_adc_ch3.calibrate = ads1115_ch_info->calibrate;
		acs71x_info_adc_ch3.device_info = &device_info;

		const uint8_t range = acs71x_get_range(&acs71x_info_adc_ch3);

		rdm_sensor_acs71x_ch3.range_min = (uint16_t) -range * 1000;
		rdm_sensor_acs71x_ch3.range_max = (uint16_t) range * 1000;

		if (acs71x_start(&acs71x_info_adc_ch3)) {
			(void) rdm_sensors_add(&rdm_sensor_acs71x_ch3, ads1115_ch3);
		}
	}

}

INITIALIZER(sensors, ads1115)
