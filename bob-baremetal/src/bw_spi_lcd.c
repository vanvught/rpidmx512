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
#include "avr_spi.h"
#else
#include "bcm2835.h"
#ifdef BARE_METAL
#include "bcm2835_spi.h"
#endif
#endif
#include "device_info.h"
#include "bw.h"
#include "bw_spi_lcd.h"

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 */
inline static void lcd_spi_setup(const device_info_t *device_info) {
#ifdef __AVR_ARCH__
#else
	bcm2835_spi_setClockDivider(2500); //100kHz
	bcm2835_spi_chipSelect(device_info->chip_select);
#endif
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @return
 */
uint8_t bw_spi_lcd_start(device_info_t *device_info) {
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (bcm2835_init() != 1)
	return BW_LCD_ERROR;
#endif
	FUNC_PREFIX(spi_begin());

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	}

	return BW_LCD_OK;
}

/**
 * @ingroup SPI-LCD
 *
 */
void bw_spi_lcd_end(void) {
	FUNC_PREFIX(spi_end());
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param line
 * @param pos
 */
void bw_spi_lcd_set_cursor(const device_info_t *device_info, const uint8_t line, const uint8_t pos) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_MOVE_CURSOR, (char) 0x00 };
	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) (((line & 0x03) << 5) | (pos & 0x1f));

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0])));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text(const device_info_t *device_info, const char *text, uint8_t length) {
	char data[BW_LCD_MAX_CHARACTERS + 2];
	uint8_t i;

	data[0] = (char) device_info->slave_address;
	data[1] = (char) BW_PORT_WRITE_DISPLAY_DATA;

	if (length > BW_LCD_MAX_CHARACTERS)
		length = BW_LCD_MAX_CHARACTERS;

	for (i = 0; i < length; i++)
		data[i + 2] = text[i];

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(data, length + 2));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text_line_1(const device_info_t *device_info, const char *text,
		const uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 0, 0);
	bw_spi_lcd_text(device_info, text, length);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text_line_2(const device_info_t *device_info, const char *text,	const uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 1, 0);
	bw_spi_lcd_text(device_info, text, length);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text_line_3(const device_info_t *device_info, const char *text,	const uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 2, 0);
	bw_spi_lcd_text(device_info, text, length);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text_line_4(const device_info_t *device_info, const char *text,	const uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 3, 0);
	bw_spi_lcd_text(device_info, text, length);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 */
void bw_spi_lcd_cls(const device_info_t *device_info) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_CLEAR_SCREEN, (char) ' ' };
	cmd[0] = (char) device_info->slave_address;

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0])));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param value
 */
void bw_spi_lcd_set_contrast(const device_info_t *device_info, const uint8_t value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_SET_CONTRAST, (char) 0x00 };
	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) value;

	lcd_spi_setup(device_info);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0])));
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param value
 */
void bw_spi_lcd_set_backlight(const device_info_t *device_info,	const uint8_t value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_SET_BACKLIGHT, (char) 0x00 };
	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) value;

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0])));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 */
void bw_spi_lcd_reinit(const device_info_t *device_info) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_REINIT_LCD, (char) ' ' };
	cmd[0] = (char) device_info->slave_address;

	lcd_spi_setup(device_info);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0])));
	udelay(1000000);
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param value
 */
void bw_spi_lcd_get_backlight(const device_info_t *device_info, uint8_t *value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_READ_CURRENT_BACKLIGHT, (char) 0x00 };
	cmd[0] = (char) (device_info->slave_address | 1);

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_transfern(cmd, sizeof(cmd) / sizeof(char)));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	*value = (uint8_t)cmd[2];
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param value
 */
void bw_spi_lcd_get_contrast(const device_info_t *device_info, uint8_t *value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_READ_CURRENT_CONTRAST, (char) 0x00 };
	cmd[0] = (char) (device_info->slave_address | 1);

	lcd_spi_setup(device_info);
	FUNC_PREFIX(spi_transfern(cmd, sizeof(cmd) / sizeof(char)));
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
	*value = (uint8_t)cmd[2];
}
