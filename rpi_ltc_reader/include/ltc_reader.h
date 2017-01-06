/**
 * @file ltc_reader.h
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


#ifndef LTC_READER_H_
#define LTC_READER_H_

#include <stdbool.h>

#include "bcm2835.h"

#define ONE_TIME_MIN        150	///< 417us/2 = 208us
#define ONE_TIME_MAX       	300	///< 521us/2 = 260us
#define ZERO_TIME_MIN      	380	///< 30 FPS * 80 bits = 2400Hz, 1E6/2400Hz = 417us
#define ZERO_TIME_MAX      	600	///< 24 FPS * 80 bits = 1920Hz, 1E6/1920Hz = 521us

#define END_DATA_POSITION	63	///<
#define END_SYNC_POSITION	77	///<
#define END_SMPTE_POSITION	80	///<

#define GPIO_PIN			RPI_V2_GPIO_P1_21

typedef enum _timecode_types {
	TC_TYPE_FILM = 0,
	TC_TYPE_EBU,
	TC_TYPE_DF,
	TC_TYPE_SMPTE,
	TC_TYPE_UNKNOWN,
	TC_TYPE_INVALID = 255
} timecode_types;

struct _ltc_reader_output {
	bool console_output;
	bool lcd_output;
	bool midi_output;
	bool artnet_output;
} ltc_reader_output;

void ltc_reader(void);
void ltc_reader_init(const struct _ltc_reader_output *);

#endif /* LTC_READER_H_ */
