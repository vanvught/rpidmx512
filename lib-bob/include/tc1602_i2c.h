/**
 * @file tc1602_i2c.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef TC1602_I2C_H_
#define TC1602_I2C_H_

#define TC1602_I2C_DEFAULT_SLAVE_ADDRESS	0x27	///<

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

extern const bool tc1602_i2c_start(device_info_t *);

extern void tc1602_i2c_cls(const device_info_t *);

extern void tc1602_i2c_text(const device_info_t *, const char *, uint8_t);
extern void tc1602_i2c_text_line_1(const device_info_t *, const char *, const uint8_t);
extern void tc1602_i2c_text_line_2(const device_info_t *, const char *, const uint8_t);

#endif /* TC1602_I2C_H_ */
