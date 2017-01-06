/**
 * @file lcd.h
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

#ifndef LCD_H_
#define LCD_H_

#include <stdbool.h>

#define LCD_MAX_CHARACTERS	16
#define LCD_MAX_LINES		2

typedef enum _lcd_types {
	LCD_BW_UI = 0,
	LCD_BW_LCD,
	LCD_TC1602_PCF8574T,
	LCD_TYPE_UNKNOWN
} lcd_types;

extern const bool lcd_detect(void);

extern void lcd_cls(void);

extern void lcd_text(const char *, const uint8_t);
extern void lcd_text_line_1(const char *, const uint8_t);
extern void lcd_text_line_2(const char *, const uint8_t);

#endif /* LCD_H_ */
