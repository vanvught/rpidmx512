/**
 * @file ads1015.h
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

#ifndef ADS1015_H_
#define ADS1015_H_

#define ADS1015_DEFAULT_SLAVE_ADDRESS	0x48	///<

#define ADS1015_CH0						0x00	///<
#define ADS1015_CH1						0x01	///<
#define ADS1015_CH2						0x02	///<
#define ADS1015_CH3						0x03	///<

typedef enum ads1015_data_rate {
	ADS1015_DATA_RATE_128 = 0x0000,		///< 128 samples per second
	ADS1015_DATA_RATE_250 = 0x0020,		///< 250 samples per second
	ADS1015_DATA_RATE_490 = 0x0040,		///< 490 samples per second
	ADS1015_DATA_RATE_920 = 0x0060,		///< 920 samples per second
	ADS1015_DATA_RATE_1600 = 0x0080,	///< 1600 samples per second (default)
	ADS1015_DATA_RATE_2400 = 0x00a0,	///< 2400 samples per second
	ADS1015_DATA_RATE_3300 = 0x00c0,	///< 3300 samples per second
} ads1015_data_rate_t;

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

extern const bool ads1015_start(device_info_t *);
extern const uint16_t ads1015_read(const device_info_t *, const uint8_t);

extern const ads1015_data_rate_t ads1015_get_data_rate(const device_info_t *);
extern void ads1015_set_data_rate(const device_info_t *, const ads1015_data_rate_t);

#endif /* ADS1015_H_ */
