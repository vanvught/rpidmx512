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
#include <bw.h>
#include <bw_ui.h>

#ifndef BARE_METAL
static void uwait(int us) { bcm2835_delayMicroseconds(us); }
#else
extern void uwait(int);
#endif

#if 0
static uint64_t _st_sample;
#define BW_UI_I2C_BYTE_WAIT_US 0
inline static uint8_t _i2c_write(const char * buf, uint32_t len) {
	int delta = (int)(bcm2835_st_read() - _st_sample);
	if (delta > 0 && delta < 16)
		bcm2835_delayMicroseconds(16 - delta);
	uint8_t rc = bcm2835_i2c_write(buf, len);
	_st_sample = bcm2835_st_read();
	return rc;
}
#define bcm2835_i2c_write _i2c_write
#endif

extern int printf(const char *format, ...);

static char i2c_ui_slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;

inline static void ui_i2c_setup(void) {
	bcm2835_i2c_setSlaveAddress(i2c_ui_slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

int bw_i2c_ui_start (char slave_address) {

	if (bcm2835_init() != 1)
		return BW_UI_ERROR;

	bcm2835_i2c_begin();

	if (slave_address <= 0)
		i2c_ui_slave_address = BW_UI_DEFAULT_SLAVE_ADDRESS;
	else
		i2c_ui_slave_address = slave_address;

	return BW_UI_OK;
}

void bw_i2c_ui_end(void) {
	bcm2835_i2c_end();
	bcm2835_close();
}

void bw_i2c_ui_set_cursor(uint8_t line, uint8_t pos) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0x00 };
	if (line > BW_UI_MAX_LINES)
		line = 0;
	if (pos > BW_UI_MAX_CHARACTERS)
		pos = 0;
	cmd[1] = ((line && 0b11) << 5) | (pos && 0b11111);
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);

}

void bw_i2c_ui_text(const char *text, uint8_t length) {
	char data[BW_UI_MAX_CHARACTERS + 1] = { BW_PORT_WRITE_DISPLAY_DATA };
	if (length > BW_UI_MAX_CHARACTERS)
		length = BW_UI_MAX_CHARACTERS;
	uint8_t i;
	for (i = 0; i < length; i++)
		data[i + 1] = text[i];
	ui_i2c_setup();
	bcm2835_i2c_write(data, length + 1);
	uwait(BW_UI_I2C_BYTE_WAIT_US);
}

void bw_i2c_ui_text_line_1(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0000000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

void bw_i2c_ui_text_line_2(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b0100000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

void bw_i2c_ui_text_line_3(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1000000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

void bw_i2c_ui_text_line_4(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0b1100000 };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bw_i2c_ui_text(text, length);
}

void bw_i2c_ui_cls(void) {
	static char cmd[] = { BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
}

void bw_i2c_ui_set_contrast(uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_CONTRAST, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
}

void bw_i2c_ui_set_backlight(uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
}

void bw_i2c_ui_set_backlight_temp(uint8_t value) {
	static char cmd[] = { BW_PORT_WRITE_SET_BACKLIGHT_TEMP, 0x00 };
	cmd[1] = value;
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
}

//TODO
void bw_i2c_ui_set_startup_message_line_1(const char *text, uint8_t length) {
	static char cmd[] = { BW_PORT_WRITE_STARTUPMESSAGE_LINE1, 0xFF };
	if (length == 0) {
		ui_i2c_setup();
		uwait(BW_UI_I2C_BYTE_WAIT_US);
		bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	} else {

	}
}

void bw_i2c_ui_set_startup_message_line_2(const char *text, uint8_t length) {
}

void bw_i2c_ui_set_startup_message_line_3(const char *text, uint8_t length) {
}

void bw_i2c_ui_set_startup_message_line_4(const char *text, uint8_t length) {
}

void bw_i2c_ui_get_backlight(uint8_t *value) {
	static char cmd[] = { BW_PORT_READ_CURRENT_BACKLIGHT };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read((char *)value, 1);
}

void bw_i2c_ui_get_contrast(uint8_t *value) {
	static char cmd[] = { BW_PORT_READ_CURRENT_CONTRAST };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read((char *)value, 1);
}

void bw_i2c_ui_reinit(void) {
	static char cmd[] = { BW_PORT_WRITE_REINIT_LCD, ' ' };
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(500000);
}

void bw_i2c_ui_read_id(void) {
	static char cmd[] = { BW_PORT_READ_ID_STRING };
	char buf[BW_UI_ID_STRING_LENGTH];
	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, BW_UI_ID_STRING_LENGTH);
	printf("[%s]\r\n", buf);
}

// UI specific

char bw_i2c_ui_read_button(char button) {
	if ((button < BW_UI_BUTTON1) | (button > BW_UI_BUTTON6))
		return 0;

	static char cmd[2];
	char buf[1];

	cmd[0] = BW_PORT_READ_BUTTON_1 + button;
	cmd[1] = 0xFF;

	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, sizeof(buf) / sizeof(char));

	return (buf[0]);
}

char bw_i2c_ui_read_button_last(void) {
	static char cmd[] = {BW_PORT_READ_BUTTON_SINCE_LAST, 0xFF};
	char buf[1];

	ui_i2c_setup();
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_UI_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, sizeof(buf) / sizeof(char));

	return (buf[0]);
}
