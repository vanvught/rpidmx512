/**
 * @file bw_i2c_lcd.c
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

#include <stdint.h>

#include "bcm2835.h"
#include "bcm2835_i2c.h"

#include "bw.h"
#include "bw_lcd.h"
#include "bw_i2c_lcd.h"

#include "device_info.h"

#define BW_LCD_I2C_BYTE_WAIT_US	37

/**
 *
 * @param device_info
 */
inline static void lcd_i2c_setup(const device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @return
 */
void bw_i2c_lcd_start(device_info_t *device_info) {
	bcm2835_i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	}
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param line
 * @param pos
 */
void bw_i2c_lcd_set_cursor(const device_info_t *device_info, const uint8_t line, const uint8_t pos) {
	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0x00 };

	cmd[1] = ((line & 0x03) << 5) | (pos & 0x1f);

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_lcd_text(const device_info_t *device_info, const char *text, uint8_t length) {
	uint8_t i;
	char data[BW_LCD_MAX_CHARACTERS + 1] = { BW_PORT_WRITE_DISPLAY_DATA };

	if (length > (uint8_t) BW_LCD_MAX_CHARACTERS) {
		length = (uint8_t) BW_LCD_MAX_CHARACTERS;
	}

	for (i = 0; i < length; i++) {
		data[i + 1] = text[i];
	}

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(data, length + 1);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_1(const device_info_t *device_info, const char *text, const uint8_t length) {
	bw_i2c_lcd_set_cursor(device_info, 0, 0);
	udelay(BW_LCD_I2C_BYTE_WAIT_US);
	bw_i2c_lcd_text(device_info, text, length);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_2(const device_info_t *device_info, const char *text, const uint8_t length) {
	bw_i2c_lcd_set_cursor(device_info, 1, 0);
	udelay(BW_LCD_I2C_BYTE_WAIT_US);
	bw_i2c_lcd_text(device_info, text, length);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 */
void bw_i2c_lcd_cls(const device_info_t *device_info) {
	char cmd[] = { BW_PORT_WRITE_CLEAR_SCREEN, ' ' };

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param value
 */
void bw_i2c_lcd_set_contrast(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_CONTRAST, 0x00 };

	cmd[1] = value;
	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param value
 */
void bw_i2c_lcd_set_backlight(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };

	cmd[1] = value;
	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_1(const device_info_t *device_info, /*@unused@*/const char *text, uint8_t length) { //TODO implement
	const char cmd[] = { BW_PORT_WRITE_STARTUPMESSAGE_LINE1, 0xFF };

	if (length == (uint8_t) 0) {
		lcd_i2c_setup(device_info);
		bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	} else {

	}
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_2(const device_info_t *device_info, /*@unused@*/const char *text, uint8_t length) { //TODO implement
	const char cmd[] = { BW_PORT_WRITE_STARTUPMESSAGE_LINE2, 0xFF };

	if (length == (uint8_t) 0) {
		lcd_i2c_setup(device_info);
		bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	} else {

	}
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param value
 */
void bw_i2c_lcd_set_backlight_temp(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT_TEMP, 0x00 };

	cmd[1] = value;
	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param value
 */
void bw_i2c_lcd_get_backlight(const device_info_t *device_info, uint8_t *value) {
	const char cmd[] = { BW_PORT_READ_CURRENT_BACKLIGHT };

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	bcm2835_i2c_read((char *)value, 1);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 * @param value
 */
void bw_i2c_lcd_get_contrast(const device_info_t *device_info, uint8_t *value) {
	const char cmd[] = { BW_PORT_READ_CURRENT_CONTRAST };

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	bcm2835_i2c_read((char *)value, 1);
}

/**
 * @ingroup I2C-LCD
 *
 * @param device_info
 */
void bw_i2c_lcd_reinit(const device_info_t *device_info) {
	const char cmd[] = { BW_PORT_WRITE_REINIT_LCD, ' ' };

	lcd_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
}
