#if defined(HAVE_I2C)
/**
 * @file tc1602_i2c.c
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

extern void udelay(uint32_t);

#if defined(__linux__) || defined(__circle__)
 #define udelay bcm2835_delayMicroseconds
#endif

#include "i2c.h"
#include "tc1602.h"
#include "tc1602_i2c.h"
#include "device_info.h"

static void i2c_setup(const device_info_t *device_info) {
	i2c_set_address(device_info->slave_address);

	if (device_info->fast_mode) {
		i2c_set_baudrate(I2C_FULL_SPEED);
	} else {
		i2c_set_baudrate(I2C_NORMAL_SPEED);
	}
}

static void lcd_toggle_enable(const uint8_t data) {
	i2c_write(data | TC1602_EN | TC1602_BACKLIGHT);
	i2c_write((data & ~TC1602_EN) | TC1602_BACKLIGHT);
}

static void write_4bits(const device_info_t *device_info, const uint8_t data) {
	i2c_setup(device_info);

	i2c_write(data);
	lcd_toggle_enable(data);
}

static void write_cmd(const device_info_t *device_info, const uint8_t cmd) {
	write_4bits(device_info, cmd & (uint8_t) 0xF0);
	write_4bits(device_info, (cmd << 4) & (uint8_t) 0xF0);
	udelay(EXEC_TIME_CMD);
}

static void write_reg(const device_info_t *device_info, const uint8_t reg) {
	write_4bits(device_info, (uint8_t) TC1602_RS | (reg & (uint8_t) 0xF0));
	write_4bits(device_info, (uint8_t) TC1602_RS | ((reg << 4) & (uint8_t) 0xF0));
	udelay(EXEC_TIME_REG);
}

const bool tc1602_i2c_start(device_info_t *device_info) {

	i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = TC1602_I2C_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->fast_mode = true;
	}

	i2c_setup(device_info);

	if (!i2c_is_connected(device_info->slave_address)) {
		return false;
	}

	write_cmd(device_info, (uint8_t) 0x33);	///< 110011 Initialize
	write_cmd(device_info, (uint8_t) 0x32);	///< 110010 Initialize

	write_cmd(device_info, (uint8_t) (TC1602_IC_FUNC | TC1602_IC_FUNC_4BIT | TC1602_IC_FUNC_2LINE | TC1602_IC_FUNC_5x8DOTS)); ///< Data length, number of lines, font size
	write_cmd(device_info, (uint8_t) (TC1602_IC_DISPLAY | TC1602_IC_DISPLAY_ON | TC1602_IC_DISPLAY_CURSOR_OFF | TC1602_IC_DISPLAY_BLINK_OFF));	///< Display On,Cursor Off, Blink Off
	write_cmd(device_info, (uint8_t) TC1602_IC_CLS);
	udelay(EXEC_TIME_CLS - EXEC_TIME_CMD);
	write_cmd(device_info, (uint8_t) (TC1602_IC_ENTRY_MODE | TC1602_IC_ENTRY_MODE_INC));	///< Cursor move direction

	return true;
}

void tc1602_i2c_text(const device_info_t *device_info, const char *data, uint8_t length) {
	uint8_t i;

	if (length > 16) {
		length = 16;
	}

	for (i = 0; i < length; i++) {
		write_reg(device_info, (uint8_t) data[i]);
	}

}

void tc1602_i2c_text_line_1(const device_info_t *device_info, const char *data, const uint8_t length) {
	write_cmd(device_info, TC1602_LINE_1);
	tc1602_i2c_text(device_info, data, length);
}

void tc1602_i2c_text_line_2(const device_info_t *device_info, const char *data, const uint8_t length) {
	write_cmd(device_info, TC1602_LINE_2);
	tc1602_i2c_text(device_info, data, length);
}

void tc1602_i2c_cls(const device_info_t *device_info) {
	write_cmd(device_info, (uint8_t) TC1602_IC_CLS);
	udelay(EXEC_TIME_CLS - EXEC_TIME_CMD);
}
#endif
