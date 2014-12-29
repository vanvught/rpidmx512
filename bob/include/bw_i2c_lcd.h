/**
 * @file bw_i2c_lcd.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BW_I2C_LCD_H_
#define BW_I2C_LCD_H_

#include <stdint.h>

#include <bw_lcd.h>
#include <device_info.h>

#define BW_LCD_I2C_BYTE_WAIT_US			12

extern uint8_t bw_i2c_lcd_start (const char);
extern void bw_i2c_lcd_end (void);
extern void bw_i2c_lcd_reinit(void);
extern void bw_i2c_lcd_set_cursor(uint8_t, uint8_t);
extern void bw_i2c_lcd_text(const char *, uint8_t);
extern void bw_i2c_lcd_text_line_1(const char *, const uint8_t);
extern void bw_i2c_lcd_text_line_2(const char *, const uint8_t);
extern void bw_i2c_lcd_text_line_3(const char *, const uint8_t);
extern void bw_i2c_lcd_text_line_4(const char *, const uint8_t);
extern void bw_i2c_lcd_cls(void);
extern void bw_i2c_lcd_set_contrast(const uint8_t);
extern void bw_i2c_lcd_set_backlight(const uint8_t);
extern void bw_i2c_lcd_get_backlight(uint8_t *);
extern void bw_i2c_lcd_get_contrast(uint8_t *);

#endif /* BW_I2C_LCD_H_ */
