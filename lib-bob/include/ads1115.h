/**
 * @file ads1115.h
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

#ifndef ADS1115_H_
#define ADS1115_H_

#define ADS1115_DEFAULT_SLAVE_ADDRESS	0x48	///<

#define ADS1115_CH0						0x00	///<
#define ADS1115_CH1						0x01	///<
#define ADS1115_CH2						0x02	///<
#define ADS1115_CH3						0x03	///<

#define ADS1115_MV_6P144    			((float) 0.187500)
#define ADS1115_MV_4P096  				((float) 0.125000)
#define ADS1115_MV_2P048				((float) 0.062500)
#define ADS1115_MV_1P024			    ((float) 0.031250)
#define ADS1115_MV_0P512  				((float) 0.015625)
#define ADS1115_MV_0P256  			  	((float) 0.007813)
#define ADS1115_MV_0P256B   			((float) 0.007813)
#define ADS1115_MV_0P256C				((float) 0.007813)

typedef enum ads1115_data_rate {
	ADS1115_DATA_RATE_8 = 0x0000,		///< 8 samples per second
	ADS1115_DATA_RATE_16 = 0x0020,		///< 16 samples per second
	ADS1115_DATA_RATE_32 = 0x0040,		///< 32 samples per second
	ADS1115_DATA_RATE_64 = 0x0060,		///< 64 samples per second
	ADS1115_DATA_RATE_128 = 0x0080,		///< 128 samples per second (default)
	ADS1115_DATA_RATE_250 = 0x00a0,		///< 250 samples per second
	ADS1115_DATA_RATE_475 = 0x00c0,		///< 475 samples per second
	ADS1115_DATA_RATE_860 = 0x00e0		///< 860 samples per second
} ads1115_data_rate_t;

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

extern bool ads1115_start(device_info_t *);
extern uint16_t ads1115_read(const device_info_t *, const uint8_t);

extern ads1115_data_rate_t ads1115_get_data_rate(const device_info_t *);
extern void ads1115_set_data_rate(const device_info_t *, const ads1115_data_rate_t);

#endif /* ADS1115_H_ */
