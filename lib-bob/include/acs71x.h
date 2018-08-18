/**
 * @file acs71x.h
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

#ifndef ACS71X_H_
#define ACS71X_H_

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

typedef enum acs71x_type {
	ACS712_05B,
	ACS712_20A,
	ACS712_30A,
	ACS711_15,
	ACS711_31
} acs71x_type_t;

typedef enum acs71x_adc {
	ACS71X_ADC_PCF8591,	//  8-bit ADC
	ACS71X_ADC_BW_UI,	// 10-bit ADC
	ACS71X_ADC_ADS1115	// 16-bit ADC
} acs71x_adc_t;

typedef enum acs712_adc_ch {
	ACS71X_ADC_CH0,
	ACS71X_ADC_CH1,
	ACS71X_ADC_CH2,
	ACS71X_ADC_CH3
} acs71x_adc_ch_t;

typedef struct _acs71x_info {
	acs71x_type_t type;
	acs71x_adc_t adc;
	acs71x_adc_ch_t channel;
	bool calibrate;
	uint16_t zero;
	device_info_t *device_info;
} acs71x_info_t;

extern bool acs71x_start(acs71x_info_t *);
extern uint16_t acs71x_calibrate(const acs71x_info_t *);
extern int16_t acs71x_get_current_dc(const acs71x_info_t *);

extern /*@shared@*//*@null@*/const char *acs71x_get_chip_name(const acs71x_info_t *);
extern /*@shared@*//*@null@*/const char *acs71x_get_adc_name(const acs71x_info_t *);
extern uint8_t acs71x_get_range(const acs71x_info_t *);

#endif /* ACS71X_H_ */
