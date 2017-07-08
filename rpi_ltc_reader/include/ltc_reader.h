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
	bool oled_output;
	bool segment_output;
	bool midi_output;
	bool artnet_output;
	bool matrix_output;
};

#define TC_CODE_MAX_LENGTH	11
#define TC_TYPE_MAX_LENGTH	11

#ifdef __cplusplus
extern "C" {
#endif

extern void ltc_reader(void);
extern void ltc_reader_init(const struct _ltc_reader_output *);
extern /*@shared@*/const char *ltc_reader_get_type(const timecode_types);

#ifdef __cplusplus
}
#endif

#endif /* LTC_READER_H_ */
