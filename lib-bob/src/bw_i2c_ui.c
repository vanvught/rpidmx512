/**
 * @file bw_i2c_ui.c
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
#include "bw_ui.h"
#include "bw_i2c_ui.h"

#include "device_info.h"

#define BW_UI_I2C_BYTE_WAIT_US			14


/**
 * @ingroup I2C-UI
 *
 * @param device_info
 */
inline static void ui_i2c_setup(const device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

/**
 * @ingroup I2C-UI
 *
 * @param device_info
 * @return
 */
void bw_i2c_ui_start(device_info_t *device_info) {
	bcm2835_i2c_begin();

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;
	}
}

/**
 * @ingroup I2C-UI
 *
 * @param line
 * @param pos
 */
void bw_i2c_ui_set_cursor(const device_info_t *device_info, const uint8_t line, const uint8_t pos) {
	char cmd[] = { (char) BW_PORT_WRITE_MOVE_CURSOR, (char) 0x00 };
	cmd[1] = (char) (((line & 0x03) << 5) | (pos & 0x1f));

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text(const device_info_t *device_info, const char *text, uint8_t length) {
	char data[BW_UI_MAX_CHARACTERS + 1];
	uint8_t i;

	data[0] = (char) BW_PORT_WRITE_DISPLAY_DATA;

	if (length > BW_UI_MAX_CHARACTERS) {
		length = BW_UI_MAX_CHARACTERS;
	}

	for (i = 0; i < length; i++) {
		data[i + 1] = text[i];
	}

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(data, length + 1);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */

void bw_i2c_ui_text_line_1(const device_info_t *device_info, const char *text, const uint8_t length) {
	bw_i2c_ui_set_cursor(device_info, 0, 0);
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(device_info, text, length);
}

/**
 * @ingroup I2C-UI
 *
 * @param text
 * @param length
 */
void bw_i2c_ui_text_line_2(const device_info_t *device_info, const char *text, const uint8_t length) {
	bw_i2c_ui_set_cursor(device_info, 1, 0);
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(device_info, text, length);
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_cls(const device_info_t *device_info) {
	const char cmd[] = { (char) BW_PORT_WRITE_CLEAR_SCREEN, (char) ' ' };

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_contrast(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { (char) BW_PORT_WRITE_SET_CONTRAST, (char) 0x00 };
	cmd[1] = (char) value;

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_backlight(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { (char) BW_PORT_WRITE_SET_BACKLIGHT, (char) 0x00 };
	cmd[1] = (char) value;

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_set_backlight_temp(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { (char) BW_PORT_WRITE_SET_BACKLIGHT_TEMP, (char) 0x00 };
	cmd[1] = (char) value;

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

/**
 * @ingroup I2C-UI
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_ui_set_startup_message_line_1(const device_info_t *device_info, /*@unused@*/const char *text, uint8_t length) { //TODO implement
	char cmd[] = { (char) BW_PORT_WRITE_STARTUPMESSAGE_LINE1, (char) 0xFF };

	if (length == (uint8_t) 0) {
		ui_i2c_setup(device_info);
		(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {

	}
}

/**
 * @ingroup I2C-UI
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_i2c_ui_set_startup_message_line_2(const device_info_t *device_info, /*@unused@*/const char *text, uint8_t length) { //TODO implement
	char cmd[] = { (char) BW_PORT_WRITE_STARTUPMESSAGE_LINE2, (char) 0xFF };

	if (length == (uint8_t) 0) {
		ui_i2c_setup(device_info);
		(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {

	}
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_get_backlight(const device_info_t *device_info, uint8_t *value) {
	const char cmd[] = { (char) BW_PORT_READ_CURRENT_BACKLIGHT };

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	(void) bcm2835_i2c_read((char *) value, 1);
}

/**
 * @ingroup I2C-UI
 *
 * @param value
 */
void bw_i2c_ui_get_contrast(const device_info_t *device_info, uint8_t *value) {
	const char cmd[] = { (char) BW_PORT_READ_CURRENT_CONTRAST };

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	(void) bcm2835_i2c_read((char *) value, 1);
}

/**
 * @ingroup I2C-UI
 *
 */
void bw_i2c_ui_reinit(const device_info_t *device_info) {
	char cmd[] = { (char) BW_PORT_WRITE_REINIT_LCD, (char) ' ' };
	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(500000);
}

// UI specific
/**
 * @ingroup I2C-UI
 *
 * @param button
 * @return
 */
char bw_i2c_ui_read_button(const device_info_t *device_info, const BwUiButtons button) {
	char cmd[2];
	char buf[1];

	if ((button < BW_UI_BUTTON1) | (button > BW_UI_BUTTON6)) {
		return (char) 0;
	}

	cmd[0] = (char) BW_PORT_READ_BUTTON_1 + (char) button;
	cmd[1] = (char) 0xFF;

	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	(void) bcm2835_i2c_read(buf, sizeof(buf) / sizeof(buf[0]));

	return (buf[0]);
}

/**
 * @ingroup I2C-UI
 *
 * @return
 */
char bw_i2c_ui_read_button_last(const device_info_t *device_info) {
	char cmd[] = { (char) BW_PORT_READ_BUTTON_SINCE_LAST, (char) 0xFF };
	char buf[1];
	ui_i2c_setup(device_info);
	(void) bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(BW_UI_I2C_BYTE_WAIT_US);
	(void) bcm2835_i2c_read(buf, sizeof(buf) / sizeof(buf[0]));
	return (buf[0]);
}
