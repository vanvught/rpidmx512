/**
 * @file bw_i2c_ui.c
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

#include <bcm2835.h>
#include <device_info.h>
#include <bw.h>
#include <bw_i2c_ui.h>

#ifndef BARE_METAL
#define udelay bcm2835_delayMicroseconds
#endif

extern int printf(const char *format, ...);

static char i2c_ui_slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;

/**
 * @ingroup I2C-UI
 *
 */
inline static void ui_i2c_setup(void) {
	bcm2835_i2c_setSlaveAddress(i2c_ui_slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

/**
 * @ingroup I2C-UI
 *
 * @param slave_address
 * @return
 */
int bw_i2c_ui_start (const char slave_address) {

	if (bcm2835_init() != 1)
		return BW_UI_ERROR;

	bcm2835_i2c_begin();

	if (slave_address <= 0)
		i2c_ui_slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;
	else
		i2c_ui_slave_address = slave_address;

	return BW_UI_OK;
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_end(void) {
	bcm2835_i2c_end();
	bcm2835_close();
}

/**
 * @ingroup I2C-UI
 *
 * @param line
 * @param pos
 */
void bw_i2c_ui_set_cursor(uint8_t line, uint8_t pos) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0x00 };
	if (line > BW_UI_MAX_LINES)
		line = 0;
	if (pos > BW_UI_MAX_CHARACTERS)
		pos = 0;
	cmd[1] = ((line && 0b11) << 5) | (pos && 0b11111);
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);

}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text(const char *text, uint8_t length) {
	char data[BW_UI_MAX_CHARACTERS + 1] = { BW_PORT_WRITE_DISPLAY_DATA };
	if (length > BW_UI_MAX_CHARACTERS)
		length = BW_UI_MAX_CHARACTERS;
	uint8_t i;
	for (i = 0; i < length; i++)
		data[i + 1] = text[i];
	ui_i2c_setup();
	bcm2835_i2c_write(data, length + 1);
	udelay(BW_UI_I2C_BYTE_WAIT_US);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */

void bw_i2c_ui_text_line_1(const char *text, const uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0000000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text_line_2(const char *text, const uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0100000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text_line_3(const char *text, const uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1000000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text_line_4(const char *text, const uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1100000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_cls(void) {
	static char cmd[] = { BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_contrast(const uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_CONTRAST, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_backlight(const uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_backlight_temp(const uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT_TEMP, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
}

//TODO
void bw_i2c_ui_set_startup_message_line_1(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_STARTUPMESSAGE_LINE1, 0xFF };
	if (length == 0) {
		ui_i2c_setup();
		udelay(BW_UI_I2C_BYTE_WAIT_US);
		bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	} else {

	}
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_set_startup_message_line_2(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_set_startup_message_line_3(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_set_startup_message_line_4(const char *text, uint8_t length) {
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_get_backlight(uint8_t *value) {
	static char cmd[] = { BW_PORT_READ_CURRENT_BACKLIGHT };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read((char *)value, 1);
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_get_contrast(uint8_t *value) {
	static char cmd[] = { BW_PORT_READ_CURRENT_CONTRAST };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read((char *)value, 1);
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_reinit(void) {
	static char cmd[] = { BW_PORT_WRITE_REINIT_LCD, ' ' };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(500000);
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_read_id(void) {
	static char cmd[] = { BW_PORT_READ_ID_STRING };
	char buf[BW_ID_STRING_LENGTH];
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, BW_ID_STRING_LENGTH);
	printf("[%s]\n", buf);
}

// UI specific
/**
 * @ingroup I2C-UI
 *
 * @param button
 * @return
 */
char bw_i2c_ui_read_button(const char button) {
	if ((button < BW_UI_BUTTON1) | (button > BW_UI_BUTTON6))
		return 0;

	static char cmd[2];
	char buf[1];

	cmd[0] = BW_PORT_READ_BUTTON_1 + button;
	cmd[1] = 0xFF;

	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, sizeof(buf) / sizeof(char));

	return (buf[0]);
}

/**
 * @ingroup I2C-UI
 *
 * @return
 */
char bw_i2c_ui_read_button_last(void) {
	static char cmd[] = {BW_PORT_READ_BUTTON_SINCE_LAST, 0xFF};
	char buf[1];

	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, sizeof(buf) / sizeof(char));

	return (buf[0]);
}
