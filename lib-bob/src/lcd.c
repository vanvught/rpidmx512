#if defined(HAVE_I2C)
/**
 * @file lcd.c
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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#include "i2c.h"

#include "lcd.h"

#include "bw.h"
#include "bw_i2c_ui.h"
#include "bw_i2c_lcd.h"
#include "tc1602.h"
#include "tc1602_i2c.h"

#include "device_info.h"

#if defined(__linux__)
extern void bcm2835_delayMicroseconds (const uint64_t);
#define udelay bcm2835_delayMicroseconds
#else
extern void udelay(uint32_t);
#endif

#ifndef ALIGNED
 #define ALIGNED	__attribute__((aligned(4)))
#endif

static const char lcd_detect_line1[] ALIGNED = "lcd_detect";
#define LCD_DETECT_LINE1_LENGTH			(sizeof (lcd_detect_line1) / sizeof(char)) - 1

static const char lcd_detect_line2_bw_ui[] ALIGNED = "bw_i2c_ui";
#define LCD_DETECT_LINE2_BW_UI_LENGTH	(sizeof (lcd_detect_line2_bw_ui) / sizeof(char)) - 1

static const char lcd_detect_line2_bw_lcd[] ALIGNED = "bw_i2c_lcd";
#define LCD_DETECT_LINE2_BW_LCD_LENGTH	(sizeof (lcd_detect_line2_bw_lcd) / sizeof(char)) - 1

static const char lcd_detect_line2_tc1602[] ALIGNED = "tc1602_i2c_lcd";
#define LCD_DETECT_LINE2_TC1602_LENGTH	(sizeof (lcd_detect_line2_tc1602) / sizeof(char)) - 1

static device_info_t device_info = { (uint8_t) 0, (uint8_t) 0, (uint32_t) 0, (uint16_t) 0 };

static void _lcd_cls(/*@unused@*/const device_info_t *device_info) {}
static void _lcd_text(/*@unused@*/const device_info_t *device_info, /*@unused@*/const char *text, /*@unused@*/uint8_t length) {}
static void _lcd_text_line_1(/*@unused@*/const device_info_t *device_info, /*@unused@*/const char *text, /*@unused@*/uint8_t length) {}
static void _lcd_text_line_2(/*@unused@*/const device_info_t *device_info, /*@unused@*/const char *text, /*@unused@*/uint8_t length) {}

static void(*lcd_cls_f)(const device_info_t *) = _lcd_cls;
static void(*lcd_text_f)(const device_info_t *, const char *, uint8_t) = _lcd_text;
static void(*lcd_text_line_1_f)(const device_info_t *, const char *, uint8_t) = _lcd_text_line_1;
static void(*lcd_text_line_2_f)(const device_info_t *, const char *, uint8_t) = _lcd_text_line_2;

const bool lcd_detect(void) {
	device_info.chip_select = (uint8_t) 0;
	device_info.slave_address = (uint8_t) 0;
	device_info.speed_hz = (uint32_t) 0;

	lcd_cls_f = _lcd_cls;
	lcd_text_f = _lcd_text;
	lcd_text_line_1_f = _lcd_text_line_1;
	lcd_text_line_2_f = _lcd_text_line_2;

	if (!i2c_begin()) {
		return false;
	}
	i2c_set_clockdivider(I2C_CLOCK_DIVIDER_100kHz);

	if (i2c_is_connected(BW_UI_DEFAULT_SLAVE_ADDRESS >> 1)) {
		device_info.slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;
		bw_i2c_ui_start(&device_info);
		bw_i2c_ui_set_backlight(&device_info, 64);
		udelay(100);
		bw_i2c_ui_set_contrast(&device_info, 64);
		udelay(100);
		//
		lcd_cls_f = bw_i2c_ui_cls;
		lcd_text_f = bw_i2c_ui_text;
		lcd_text_line_1_f = bw_i2c_ui_text_line_1;
		lcd_text_line_2_f = bw_i2c_ui_text_line_2;
		//
		lcd_text_line_1_f(&device_info, lcd_detect_line1, LCD_DETECT_LINE1_LENGTH);
		lcd_text_line_2_f(&device_info, lcd_detect_line2_bw_ui, LCD_DETECT_LINE2_BW_UI_LENGTH);
		return true;
	}

	if (i2c_is_connected(BW_LCD_DEFAULT_SLAVE_ADDRESS >> 1)) {
		device_info.slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
		bw_i2c_lcd_start(&device_info);
		bw_i2c_lcd_set_backlight(&device_info, 64);
		udelay(100);
		bw_i2c_lcd_set_contrast(&device_info, 64);
		udelay(100);
		//
		lcd_cls_f = bw_i2c_lcd_cls;
		lcd_text_f = bw_i2c_lcd_text;
		lcd_text_line_1_f = bw_i2c_lcd_text_line_1;
		lcd_text_line_2_f = bw_i2c_lcd_text_line_2;
		//
		lcd_text_line_1_f(&device_info, lcd_detect_line1, LCD_DETECT_LINE1_LENGTH);
		lcd_text_line_2_f(&device_info, lcd_detect_line2_bw_lcd, LCD_DETECT_LINE2_BW_LCD_LENGTH);
		return true;
	}


	if (i2c_is_connected(TC1602_I2C_DEFAULT_SLAVE_ADDRESS)) {
		device_info.slave_address = TC1602_I2C_DEFAULT_SLAVE_ADDRESS;
		tc1602_i2c_start(&device_info);
		//
		lcd_cls_f = tc1602_i2c_cls;
		lcd_text_f = tc1602_i2c_text;
		lcd_text_line_1_f = tc1602_i2c_text_line_1;
		lcd_text_line_2_f = tc1602_i2c_text_line_2;
		//
		lcd_text_line_1_f(&device_info, lcd_detect_line1, LCD_DETECT_LINE1_LENGTH);
		lcd_text_line_2_f(&device_info, lcd_detect_line2_tc1602, LCD_DETECT_LINE2_TC1602_LENGTH);
		return true;
	}

	return false;
}

void lcd_cls(void) {
	lcd_cls_f(&device_info);
}

void lcd_text(const char *text, uint8_t length) {
	lcd_text_f(&device_info, text, length);
}

void lcd_text_line_1(const char *text, uint8_t length) {
	lcd_text_line_1_f(&device_info, text, length);
}

void lcd_text_line_2(const char *text, uint8_t length) {
	lcd_text_line_2_f(&device_info, text, length);
}

int lcd_printf_line_1(const char *format, ...) {
	int i;
	char buffer[LCD_MAX_CHARACTERS + 1];

	va_list arp;
	va_start(arp, format);
	i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);
	va_end(arp);
	lcd_text_line_1_f(&device_info, buffer, i);

	return i;
}

int lcd_printf_line_2(const char *format, ...) {
	int i;
	char buffer[LCD_MAX_CHARACTERS + 1];

	va_list arp;
	va_start(arp, format);
	i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);
	va_end(arp);
	lcd_text_line_2_f(&device_info, buffer, i);

	return i;
}
#endif
