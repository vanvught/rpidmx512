/**
 * @file acs71x.c
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
#include <stdbool.h>
#include <stddef.h>

#include "acs71x.h"

#include "device_info.h"

#include "pcf8591.h"		///< 8-bit
#include "bw_i2c_ui.h"		///< 10-bit
#include "ads1115.h"		///< 16-bit

static const char acs71x_name[5][14] __attribute__((aligned(4))) = {"ACS712ELC-05\0", "ACS712ELC-20B", "ACS712ELC-30A", "ACS711EX-15\0\0\0", "ACS711EX-31\0\0\0"};
static const uint8_t acs71x_range[5] __attribute__((aligned(4))) = { 5, 20, 30, 15, 31};
static const uint16_t acs71x_sensitivity_mv[5] = {185, 100, 66, 136, 68};

enum adc_range {
	ADC_RANGE_8BIT = ((1 << 8) -1),		///<
	ADC_RANGE_10BIT = ((1 << 10) -1),	///<
	ADC_RANGE_12BIT = ((1 << 12) -1),	///<
	ADC_RANGE_15BIT = ((1 << 15) -1),	///< 16-bit ADC single-ended. Hence half the range.
};

enum adc_vref_mv {
	ADC_VREF_4V9_MV = 4900,	///< Used with BitWizard ADC
	ADC_VREF_5V_MV  = 5000,	///< Used with PCF8591
	ADC_VREF_6V1_MV = 6144	///< Used with ADS1015, ADS1115
};

/*
 * Wrapper
 */
static uint16_t uint16_bw_i2c_ui_read_adc(const device_info_t *device_info, /*@unused@*/uint8_t channel) {
	return (uint16_t) bw_i2c_ui_read_adc(device_info);
}

/*
 * Wrapper
 */
static uint16_t uint16_pcf8591_adc_read(const device_info_t *device_info, uint8_t channel) {
	return (uint16_t) pcf8591_adc_read(device_info, channel | 0x40);
}

struct _adc_device {
	/*@null@*/bool (*start)(device_info_t *);
	/*@null@*/uint16_t (*read)(const device_info_t *, uint8_t);
	const uint16_t scale;
	const uint16_t vref;
	const acs71x_adc_ch_t max_ch;
	const char name[16];
} static const _adc_device_f[] = {
		{ pcf8591_start, uint16_pcf8591_adc_read, ADC_RANGE_8BIT, ADC_VREF_5V_MV, ACS71X_ADC_CH3, "PCF8591"},			//  8-bit ADC
		{ bw_i2c_ui_start, uint16_bw_i2c_ui_read_adc, ADC_RANGE_10BIT, ADC_VREF_4V9_MV, ACS71X_ADC_CH0, "BW_UI_ADC"},	// 10-bit ADC
		{ ads1115_start, ads1115_read,  ADC_RANGE_15BIT, ADC_VREF_6V1_MV, ACS71X_ADC_CH3, "ADS1115"}					// 16-bit ADC
};

bool acs71x_start(acs71x_info_t *acs71x_info) {
	if (acs71x_info == NULL) {
		return false;
	}

	if (acs71x_info->device_info == NULL) {
		return false;
	}

	if ((acs71x_info->type < ACS712_05B) || (acs71x_info->type > ACS711_31)) {
		return false;
	}

	if (acs71x_info->adc >= (sizeof(_adc_device_f) / sizeof(_adc_device_f[0]))) {
		return false;
	}

	if ((acs71x_info->channel < ACS71X_ADC_CH0) || (acs71x_info->channel > _adc_device_f[acs71x_info->adc].max_ch)) {
		return false;
	}

	if (_adc_device_f[acs71x_info->adc].start != NULL) {
		if (_adc_device_f[acs71x_info->adc].start(acs71x_info->device_info)) {
			if (acs71x_info->calibrate) {
				acs71x_info->zero = acs71x_calibrate(acs71x_info);
			} else {
				acs71x_info->zero = (uint16_t) (((int32_t) 2500 * (int32_t) _adc_device_f[acs71x_info->adc].scale)
						/ ((int32_t) _adc_device_f[acs71x_info->adc].vref));
			}
		} else {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

/**
 *
 * @param acs712_info
 * @return
 */
const char *acs71x_get_chip_name(const acs71x_info_t *acs71x_info) {
	if (acs71x_info->type >= (sizeof(acs71x_name) / sizeof(acs71x_name[0]))) {
		return NULL;
	}

	return acs71x_name[acs71x_info->type];
}

/**
 *
 * @param acs712_info
 * @return
 */
const char *acs71x_get_adc_name(const acs71x_info_t *acs71x_info) {
	if (acs71x_info->adc >= (sizeof(_adc_device_f) / sizeof(_adc_device_f[0]))) {
		return NULL;
	}

	return _adc_device_f[acs71x_info->adc].name;
}

/**
 *
 * @param acs71x_info
 * @return
 */
uint8_t acs71x_get_range(const acs71x_info_t *acs71x_info) {
	if (acs71x_info->type >= (sizeof(acs71x_range) / sizeof(acs71x_range[0]))) {
		return 0;
	}

	return acs71x_range[acs71x_info->type];
}

/**
 *
 * @param acs712_info
 * @return
 */
uint16_t acs71x_calibrate(const acs71x_info_t *acs71x_info) {
	uint8_t i;
	uint32_t c = 0;

	for (i = 0; i < 8; i++) {
		c += (uint32_t) _adc_device_f[acs71x_info->adc].read(acs71x_info->device_info, acs71x_info->channel);
	}

	c /= 8;

	return (uint16_t) c;
}

/**
 *
 * @param acs712_info
 * @return
 */
int16_t acs71x_get_current_dc(const acs71x_info_t *acs71x_info) {
	uint8_t i;
	int32_t c = 0;
	int32_t numerator;
	int32_t denominator;

	for (i = 0; i < 8; i++) {
		c += (int32_t)((int16_t)_adc_device_f[acs71x_info->adc].read(acs71x_info->device_info, acs71x_info->channel) - (int16_t)acs71x_info->zero);
	}

	c /= 8;

	numerator = c * (int32_t) _adc_device_f[acs71x_info->adc].vref;
	denominator = (int32_t) (_adc_device_f[acs71x_info->adc].scale * acs71x_sensitivity_mv[acs71x_info->type]);

	if (_adc_device_f[acs71x_info->adc].scale < ADC_RANGE_12BIT) {
		numerator *= 1000;
	} else {
		denominator /= 1000;
	}

	return (int16_t) (numerator / denominator);
}
