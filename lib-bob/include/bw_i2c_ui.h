/**
 * @file bw_i2c_ui.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BW_I2C_UI_H_
#define BW_I2C_UI_H_

#include <stdint.h>
#include <stdbool.h>

#include "bw.h"

#include "device_info.h"

extern bool bw_i2c_ui_start(device_info_t *);
extern void bw_i2c_ui_reinit(const device_info_t *);

extern void bw_i2c_ui_cls(const device_info_t *);

extern void bw_i2c_ui_set_cursor(const device_info_t *, uint8_t, uint8_t);
extern void bw_i2c_ui_text(const device_info_t *, const char *, uint8_t);
extern void bw_i2c_ui_text_line_1(const device_info_t *, const char *, uint8_t);
extern void bw_i2c_ui_text_line_2(const device_info_t *, const char *, uint8_t);

extern void bw_i2c_ui_set_startup_message_line_1(const device_info_t *, /*@unused@*/const char *, uint8_t);
extern void bw_i2c_ui_set_startup_message_line_2(const device_info_t *, /*@unused@*/const char *, uint8_t);

extern void bw_i2c_ui_get_contrast(const device_info_t *, uint8_t *);
extern void bw_i2c_ui_set_contrast(const device_info_t *, uint8_t);

extern void bw_i2c_ui_get_backlight(const device_info_t *, uint8_t *);
extern void bw_i2c_ui_set_backlight(const device_info_t *, uint8_t);

extern char bw_i2c_ui_read_button(const device_info_t *, BwUiButtons);
extern char bw_i2c_ui_read_button_last(const device_info_t *);

extern uint16_t bw_i2c_ui_read_adc(const device_info_t *);
extern uint16_t bw_i2c_ui_read_adc_avg(const device_info_t *);

#endif /* BW_I2C_UI_H_ */
