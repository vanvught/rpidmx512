/**
 * @file ads1115_params.h
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
#ifndef ADS1115_PARAMS_H_
#define ADS1115_PARAMS_H_

#include <stdbool.h>

#include <acs71x.h>

struct _ads1115_ch_info {
	bool calibrate;			///<
	struct _ch0 {
		bool is_connected;	///<
		acs71x_type_t type; ///<
	} ch0;
	struct _ch1 {
		bool is_connected;	///<
		acs71x_type_t type; ///<
	} ch1;
	struct _ch2 {
		bool is_connected;	///<
		acs71x_type_t type; ///<
	} ch2;
	struct _ch3 {
		bool is_connected;	///<
		acs71x_type_t type; ///<
	} ch3;
};

extern /*@shared@*/const struct _ads1115_ch_info *ads1115_params_get_ch_info(void);
extern void ads1115_params_init(void);

#endif /* ADS1115_PARAMS_H_ */
