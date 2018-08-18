
/**
 * @file pcf8591.h
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


#ifndef PCF8591_H_
#define PCF8591_H_

#define PCF8591_DEFAULT_SLAVE_ADDRESS	0x48	///<

#define PCF8591_ADC_CH0		0x40	///<
#define PCF8591_ADC_CH1		0x41	///<
#define PCF8591_ADC_CH2		0x42	///<
#define PCF8591_ADC_CH3		0x43	///<

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

extern bool pcf8591_start(device_info_t *);
extern void pcf8591_dac_write(const device_info_t *, uint8_t);
extern uint8_t pcf8591_adc_read(const device_info_t *, uint8_t);

#endif /* PCF8591_H_ */
