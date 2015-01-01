/**
 * @file bw_spi_lcd.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifdef __AVR_ARCH__
#include <avr_i2c.h>
#else
#include <bcm2835.h>
#ifdef BARE_METAL
#include <FUNC_PREFIX(i2c.h>
#endif
#endif
#include <device_info.h>
#include <bw.h>
#include <bw_i2c_lcd.h>

static char i2c_lcd_slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;

/**
 *
 */
inline static void lcd_i2c_setup(void) {
	FUNC_PREFIX(i2c_setSlaveAddress(i2c_lcd_slave_address >> 1));
#ifdef __AVR_ARCH__
#else
	FUNC_PREFIX(i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500));
#endif
}

/**
 * @ingroup I2C-LCD
 *
 * @param slave_address
 * @return
 */
uint8_t bw_i2c_lcd_start (const char slave_address) {
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (FUNC_PREFIX(init() != 1))
		return BW_LCD_ERROR;
#endif
	FUNC_PREFIX(i2c_begin());

	if (slave_address <= 0)
		i2c_lcd_slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	else
		i2c_lcd_slave_address = slave_address;

	return BW_LCD_OK;
}

/**
 * @ingroup I2C-LCD
 */
void bw_i2c_lcd_end(void) {
	FUNC_PREFIX(i2c_end());
}

/**
 * @ingroup I2C-LCD
 *
 * @param line
 * @param pos
 */
void bw_i2c_lcd_set_cursor(uint8_t line, uint8_t pos) {
	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0x00 };
//	if (line > BW_LCD_MAX_LINES)
//		line = 0;
//	if (pos > BW_LCD_MAX_CHARACTERS)
//		pos = 0;
	cmd[1] = ((line && 0b11) << 5) | (pos && 0b11111);
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_text(const char *text, uint8_t length) {
	char data[BW_LCD_MAX_CHARACTERS + 1] = { BW_PORT_WRITE_DISPLAY_DATA };
	if (length > BW_LCD_MAX_CHARACTERS)
		length = BW_LCD_MAX_CHARACTERS;
	uint8_t i;
	for (i = 0; i < length; i++)
		data[i + 1] = text[i];
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(data, length + 1));
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_1(const char *text, const uint8_t length) {
//	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0000000 };
//	lcd_i2c_setup();
//	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	bw_i2c_lcd_set_cursor(0, 0);
	bw_i2c_lcd_text(text, length);
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_2(const char *text, const uint8_t length) {
//	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0100000 };
//	lcd_i2c_setup();
//	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	bw_i2c_lcd_set_cursor(1, 0);
	bw_i2c_lcd_text(text, length);
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_3(const char *text, const uint8_t length) {
//	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1000000 };
//	lcd_i2c_setup();
//	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	bw_i2c_lcd_set_cursor(2, 0);
	bw_i2c_lcd_text(text, length);
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_text_line_4(const char *text, const uint8_t length) {
//	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1100000 };
//	lcd_i2c_setup();
//	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	bw_i2c_lcd_set_cursor(3, 0);
	bw_i2c_lcd_text(text, length);
}

/**
 * @ingroup I2C-LCD
 *
 */
void bw_i2c_lcd_cls(void) {
	char cmd[] = { BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	lcd_i2c_setup();
	udelay(BW_LCD_I2C_BYTE_WAIT_US);
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
}

/**
 * @ingroup I2C-LCD
 *
 * @param value
 */
void bw_i2c_lcd_set_contrast(const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_CONTRAST, 0x00 };
	cmd[1] = value;
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
}

/**
 * @ingroup I2C-LCD
 *
 * @param value
 */
void bw_i2c_lcd_set_backlight(const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[1] = value;
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
}

//TODO
/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_1(const char *text, uint8_t length) {
	char cmd[] = { BW_PORT_WRITE_STARTUPMESSAGE_LINE1, 0xFF };
	if (length == 0) {
		lcd_i2c_setup();
		FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	} else {

	}
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_2(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_3(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-LCD
 *
 * @param text
 * @param length
 */
void bw_i2c_lcd_set_startup_message_line_4(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-LCD
 *
 * @param value
 */
void bw_i2c_lcd_set_backlight_temp(const uint8_t value) {
	char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT_TEMP, 0x00 };
	cmd[1] = value;
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
}

/**
 * @ingroup I2C-LCD
 *
 * @param value
 */
void bw_i2c_lcd_get_backlight(uint8_t *value) {
	char cmd[] = { BW_PORT_READ_CURRENT_BACKLIGHT };
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	FUNC_PREFIX(i2c_read((char *)value, 1));
}

/**
 * @ingroup I2C-LCD
 *
 * @param value
 */
void bw_i2c_lcd_get_contrast(uint8_t *value) {
	char cmd[] = { BW_PORT_READ_CURRENT_CONTRAST };
	lcd_i2c_setup();
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	FUNC_PREFIX(i2c_read((char *)value, 1));
}

/**
 * @ingroup I2C-LCD
 *
 */
void bw_i2c_lcd_reinit(void) {
	char cmd[] = { BW_PORT_WRITE_REINIT_LCD, ' ' };
	lcd_i2c_setup();
	udelay(BW_LCD_I2C_BYTE_WAIT_US);
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	udelay(500000);
}
