/**
 * @file bw_spi_lcd.c
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
#ifdef BARE_METAL
#include <bcm2835_spi.h>
#endif
#include <device_info.h>
#include <bw.h>
#include <bw_spi_lcd.h>

#ifndef BARE_METAL
#define udelay bcm2835_delayMicroseconds
#endif

extern int printf(const char *format, ...);

/**
 *
 * @param device_info
 */
inline static void lcd_spi_setup(device_info_t *device_info) {
	bcm2835_spi_setClockDivider(2500); //100kHz
	bcm2835_spi_chipSelect(device_info->chip_select);
}


int bw_spi_lcd_start(device_info_t *device_info) {

	if (bcm2835_init() != 1)
		return BW_LCD_ERROR;

	bcm2835_spi_begin();

	if (device_info->slave_address <= 0)
		device_info->slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;

	return BW_LCD_OK;
}

void bw_spi_lcd_end(void) {
	bcm2835_spi_end();
	bcm2835_close();
}

void bw_spi_lcd_set_cursor(device_info_t *device_info, uint8_t line, uint8_t pos) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0x00 };
	cmd[0] = device_info->slave_address;
	if (line > BW_LCD_MAX_LINES)
		line = 0;
	if (pos > BW_LCD_MAX_CHARACTERS)
		pos = 0;
	cmd[2] = ((line && 0b11) << 5) | (pos && 0b11111);
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

void bw_spi_lcd_text(device_info_t *device_info, const char *text, uint8_t length) {
	static char data[22];
	data[0] = device_info->slave_address;
	data[1] = BW_PORT_WRITE_DISPLAY_DATA;
	if (length > BW_LCD_MAX_CHARACTERS)
		length = BW_LCD_MAX_CHARACTERS;
	uint8_t i;
	for (i = 0; i < length; i++)
		data[i + 2] = text[i];
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(data, length + 2);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

void bw_spi_lcd_text_line_1(device_info_t *device_info, const char *text, const uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b0000000 };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_2(device_info_t *device_info, const char *text, const uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b0100000 };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_3(device_info_t *device_info, const char *text, const uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b1000000 };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_4(device_info_t *device_info, const char *text, const uint8_t length) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_MOVE_CURSOR, 0b1100000 };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_cls(device_info_t *device_info) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

void bw_spi_lcd_set_contrast(device_info_t *device_info, const uint8_t value) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[0] = device_info->slave_address;
	cmd[2] = value;
	lcd_spi_setup(device_info);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
}

void bw_spi_lcd_set_backlight(device_info_t *device_info, const uint8_t value) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_SET_BACKLIGHT, 0x00 };
	cmd[0] = device_info->slave_address;
	cmd[2] = value;
	lcd_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

void bw_spi_lcd_reinit(device_info_t *device_info) {
	static char cmd[] = { 0x00, BW_PORT_WRITE_REINIT_LCD, ' ' };
	cmd[0] = device_info->slave_address;
	lcd_spi_setup(device_info);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	udelay(1000000);
}

void bw_spi_lcd_read_id(device_info_t *device_info) {
	char buf[BW_ID_STRING_LENGTH];
	int i = 0;
	for (i = 0; i < BW_ID_STRING_LENGTH; i++) {
		buf[i] = '\0';
	}

	buf[0] = device_info->slave_address | 1;
	buf[1] = BW_PORT_READ_ID_STRING;

	lcd_spi_setup(device_info);
	bcm2835_spi_setClockDivider(5000); // 50 kHz
	bcm2835_spi_transfern(buf, BW_ID_STRING_LENGTH);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	printf("[%.20s]\n", &buf[2]);
}
