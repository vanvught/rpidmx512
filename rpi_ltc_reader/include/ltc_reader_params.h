/**
 * @file ltc_reader_params.h
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

#ifndef LTC_READER_PARAMS_H_
#define LTC_READER_PARAMS_H_

#include <stdbool.h>

typedef enum ltc_reader_source {
	LTC_READER_SOURCE_LTC,
	LTC_READER_SOURCE_ARTNET,
	LTC_READER_SOURCE_MIDI
} ltc_reader_source_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const bool ltc_reader_params_is_console_output(void);
extern const bool ltc_reader_params_is_lcd_output(void);
extern const bool ltc_reader_params_is_oled_output(void);
extern const bool ltc_reader_params_is_7segment_output(void);
extern const bool ltc_reader_params_is_midi_output(void);
extern const bool ltc_reader_params_is_artnet_output(void);
extern const bool ltc_reader_params_is_matrix_output(void);

extern const ltc_reader_source_t ltc_reader_params_get_source(void);

extern const uint8_t ltc_reader_params_get_max7219_intensity(void);

extern void ltc_reader_params_init(void) ;

#ifdef __cplusplus
}
#endif

#endif /* LTC_READER_PARAMS_H_ */
