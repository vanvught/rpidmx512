/**
 * @file oled.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(__linux__) || defined(__circle__)
 #include "bcm2835.h"
 #define udelay bcm2835_delayMicroseconds
#elif defined(H3)
#else
 #include "bcm2835.h"
 #include "bcm2835_gpio.h"
 #include "bcm2835_aux_spi.h"
 #include "bcm2835_spi.h"
#endif

#include "i2c.h"
#include "oled.h"

#if defined(H3)
#else
 #define OLED_RST			RPI_V2_GPIO_P1_33	/* P1-33, GPIO13 */
 #define OLED_DC			RPI_V2_GPIO_P1_37	/* P1-37, GPIO26 */
#endif

#define OLED_FONT8x6_CHAR_W				6
#define OLED_FONT8x6_CHAR_H				8

#define SSD1306_COMMAND_MODE			0x00
#define SSD1306_DATA_MODE				0x40

#define SSD1306_LCD_WIDTH				128
//#define SSD1306_LCD_HEIGHT				64

#define SSD1306_CMD_SET_LOWCOLUMN		0x00
#define SSD1306_CMD_SET_HIGHCOLUMN		0x10
#define SSD1306_CMD_SET_MEMORYMODE		0x20
#define SSD1306_CMD_SET_COLUMNADDR		0x21
#define SSD1306_CMD_SET_PAGEADDR		0x22
#define SSD1306_CMD_SET_STARTLINE		0x40
#define SSD1306_CMD_SET_CONTRAST		0x81
#define SSD1306_CMD_SET_CHARGEPUMP		0x8D
#define SSD1306_CMD_SEGREMAP			0xA0
#define SSD1306_CMD_DISPLAY_NORMAL		0xA6
#define SSD1306_CMD_DISPLAY_INVERT		0xA7
#define SSD1306_CMD_SET_MULTIPLEX		0xA8
#define SSD1306_CMD_DISPLAY_OFF			0xAE
#define SSD1306_CMD_DISPLAY_ON			0xAF
#define SSD1306_CMD_SET_STARTPAGE		0xB0
#define SSD1306_CMD_COMSCAN_INC			0xC0
#define SSD1306_CMD_COMSCAN_DEC			0xC8
#define SSD1306_CMD_SET_DISPLAYOFFSET	0xD3
#define SSD1306_CMD_SET_DISPLAYCLOCKDIV	0xD5
#define SSD1306_CMD_SET_PRECHARGE		0xD9
#define SSD1306_CMD_SET_COMPINS			0xDA
#define SSD1306_CMD_SET_VCOMDETECT		0xDB

static uint8_t oled_font[] __attribute__((aligned(4))) = {
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00,
	0x40, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,
	0x40, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,
	0x40, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00,
	0x40, 0x36, 0x49, 0x56, 0x20, 0x50, 0x00,
	0x40, 0x00, 0x08, 0x07, 0x03, 0x00, 0x00,
	0x40, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00,
	0x40, 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00,
	0x40, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,
	0x40, 0x00, 0x80, 0x70, 0x30, 0x00, 0x00,
	0x40, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x40, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00,
	0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
	0x40, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,
	0x40, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
	0x40, 0x72, 0x49, 0x49, 0x49, 0x46, 0x00,
	0x40, 0x21, 0x41, 0x49, 0x4D, 0x33, 0x00,
	0x40, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,
	0x40, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00,
	0x40, 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00,
	0x40, 0x41, 0x21, 0x11, 0x09, 0x07, 0x00,
	0x40, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00,
	0x40, 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00,
	0x40, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x40, 0x34, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x08, 0x14, 0x22, 0x41, 0x00,
	0x40, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00,
	0x40, 0x00, 0x41, 0x22, 0x14, 0x08, 0x00,
	0x40, 0x02, 0x01, 0x59, 0x09, 0x06, 0x00,
	0x40, 0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00,
	0x40, 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00,
	0x40, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00,
	0x40, 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00,
	0x40, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,
	0x40, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x51, 0x73, 0x00,
	0x40, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,
	0x40, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,
	0x40, 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00,
	0x40, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,
	0x40, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,
	0x40, 0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00,
	0x40, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,
	0x40, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,
	0x40, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00,
	0x40, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,
	0x40, 0x26, 0x49, 0x49, 0x49, 0x32, 0x00,
	0x40, 0x03, 0x01, 0x7F, 0x01, 0x03, 0x00,
	0x40, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,
	0x40, 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00,
	0x40, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00,
	0x40, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00,
	0x40, 0x03, 0x04, 0x78, 0x04, 0x03, 0x00,
	0x40, 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00,
	0x40, 0x00, 0x7F, 0x41, 0x41, 0x41, 0x00,
	0x40, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00,
	0x40, 0x00, 0x41, 0x41, 0x41, 0x7F, 0x00,
	0x40, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00,
	0x40, 0x00, 0x03, 0x07, 0x08, 0x00, 0x00,
	0x40, 0x20, 0x54, 0x54, 0x78, 0x40, 0x00,
	0x40, 0x7F, 0x28, 0x44, 0x44, 0x38, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x44, 0x28, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x28, 0x7F, 0x00,
	0x40, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00,
	0x40, 0x00, 0x08, 0x7E, 0x09, 0x02, 0x00,
	0x40, 0x18, 0xA4, 0xA4, 0x9C, 0x78, 0x00,
	0x40, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00,
	0x40, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x00,
	0x40, 0x20, 0x40, 0x40, 0x3D, 0x00, 0x00,
	0x40, 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00,
	0x40, 0x7C, 0x04, 0x78, 0x04, 0x78, 0x00,
	0x40, 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00,
	0x40, 0xFC, 0x18, 0x24, 0x24, 0x18, 0x00,
	0x40, 0x18, 0x24, 0x24, 0x18, 0xFC, 0x00,
	0x40, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00,
	0x40, 0x48, 0x54, 0x54, 0x54, 0x24, 0x00,
	0x40, 0x04, 0x04, 0x3F, 0x44, 0x24, 0x00,
	0x40, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,
	0x40, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00,
	0x40, 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00,
	0x40, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00,
	0x40, 0x4C, 0x90, 0x90, 0x90, 0x7C, 0x00,
	0x40, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00,
	0x40, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00,
	0x40, 0x02, 0x01, 0x02, 0x04, 0x02, 0x00,
	0x40, 0x3C, 0x26, 0x23, 0x26, 0x3C, 0x00
};

static const uint8_t oled_128x64_init[] __attribute__((aligned(4))) = {
		SSD1306_CMD_DISPLAY_OFF,
		SSD1306_CMD_SET_DISPLAYCLOCKDIV, 0x80, 		// The suggested value
		SSD1306_CMD_SET_MULTIPLEX, 0x3F,			// 1/64
		SSD1306_CMD_SET_DISPLAYOFFSET, 0x00,		// No offset
		(uint8_t) SSD1306_CMD_SET_STARTLINE | 0x00,	// line #0
		SSD1306_CMD_SET_CHARGEPUMP, 0x14,
		SSD1306_CMD_SET_MEMORYMODE, 0x00,			// Horizontal addressing
		(uint8_t) SSD1306_CMD_SEGREMAP | 0x01,		// Flip horizontally
		SSD1306_CMD_COMSCAN_DEC,					// Flip vertically
		SSD1306_CMD_SET_COMPINS, 0x12,
		SSD1306_CMD_SET_CONTRAST, 0x7F,				// 0x00 to 0xFF
		SSD1306_CMD_SET_PRECHARGE, 0xF1,
		SSD1306_CMD_SET_VCOMDETECT, 0x40,
		SSD1306_CMD_DISPLAY_NORMAL,
		SSD1306_CMD_DISPLAY_ON };

static const uint8_t oled_128x32_init[] __attribute__((aligned(4))) = {
		SSD1306_CMD_DISPLAY_OFF,
		SSD1306_CMD_SET_DISPLAYCLOCKDIV, 0x80, 		// The suggested value
		SSD1306_CMD_SET_MULTIPLEX, 0x1F,			// 1/32
		SSD1306_CMD_SET_DISPLAYOFFSET, 0x00,		// No offset
		(uint8_t) SSD1306_CMD_SET_STARTLINE | 0x00,	// line #0
		SSD1306_CMD_SET_CHARGEPUMP, 0x14,
		SSD1306_CMD_SET_MEMORYMODE, 0x00,			// Horizontal addressing
		(uint8_t) SSD1306_CMD_SEGREMAP | 0x01,		// Flip horizontally
		SSD1306_CMD_COMSCAN_DEC,					// Flip vertically
		SSD1306_CMD_SET_COMPINS, 0x02,
		SSD1306_CMD_SET_CONTRAST, 0x7F,				// 0x00 to 0xFF
		SSD1306_CMD_SET_PRECHARGE, 0xF1,
		SSD1306_CMD_SET_VCOMDETECT, 0x40,
		SSD1306_CMD_DISPLAY_NORMAL,
		SSD1306_CMD_DISPLAY_ON };

static uint8_t clear_buffer[1025] __attribute__((aligned(4)));

#if !defined(H3)
static void reset(void) {
	bcm2835_gpio_set(OLED_RST);
	udelay(1000);
	bcm2835_gpio_clr(OLED_RST);
	udelay(10000);
	bcm2835_gpio_set(OLED_RST);
	udelay(10000);
}
#endif

static void i2c_setup(const oled_info_t *oled_info) {
	i2c_set_address(oled_info->slave_address);
	i2c_set_baudrate(I2C_FULL_SPEED);
	//_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_626);
}

static void _send_command(const oled_info_t *oled_info, uint8_t cmd) {
#if !defined(H3)
	char spi_data[0];


	if (oled_info->protocol == OLED_PROTOCOL_SPI) {
		bcm2835_gpio_clr(OLED_DC);
		spi_data[0] = (char) cmd;
		if (oled_info->chip_select == OLED_SPI_CS2) {
			bcm2835_aux_spi_setClockDivider(oled_info->internal.clk_div);
			bcm2835_aux_spi_transfernb(spi_data, NULL, 1);
		} else {
			bcm2835_spi_setClockDivider(oled_info->internal.clk_div);
			bcm2835_spi_chipSelect(oled_info->chip_select);
			bcm2835_spi_transfernb(spi_data, NULL, 1);
		}
	} else
#endif
	{
		i2c_setup(oled_info);
		i2c_write_reg_uint8(SSD1306_COMMAND_MODE, cmd);
	}
}

static void _send_data(const oled_info_t *oled_info, const uint8_t *data, const uint32_t len) {
#if !defined(H3)
	uint8_t *p;
	uint32_t l;

	if (oled_info->protocol == OLED_PROTOCOL_SPI) {
		p = (uint8_t *)data + 1;
		l = len - 1;
		bcm2835_gpio_set(OLED_DC);
		if (oled_info->chip_select == OLED_SPI_CS2) {
			bcm2835_aux_spi_setClockDivider(oled_info->internal.clk_div);
			bcm2835_aux_spi_transfernb((char *)p, NULL, l);
		} else {
			bcm2835_spi_setClockDivider(oled_info->internal.clk_div);
			bcm2835_spi_chipSelect(oled_info->chip_select);
			bcm2835_spi_transfernb((char *)p, NULL, l);
		}
	} else
#endif
	{
		i2c_setup(oled_info);
		(void) i2c_write_nb((const char *) data, len);
	}
}

void oled_clear(const oled_info_t *oled_info) {
	_send_command(oled_info, SSD1306_CMD_SET_COLUMNADDR);
	_send_command(oled_info, 0);						// Column start address (0 = reset)
	_send_command(oled_info, SSD1306_LCD_WIDTH - 1); 	// Column end address (127 = reset)

	switch (oled_info->type) {
	case OLED_PANEL_128x64:
		_send_command(oled_info, SSD1306_CMD_SET_PAGEADDR);
		_send_command(oled_info, 0);		// Page start address (0 = reset)
		_send_command(oled_info, 7);		// Page end address
		_send_data(oled_info, (const uint8_t *) &clear_buffer, (SSD1306_LCD_WIDTH * 64 / 8) + 1);
		break;
	case OLED_PANEL_128x32:
		_send_command(oled_info, SSD1306_CMD_SET_PAGEADDR);
		_send_command(oled_info, 0);		// Page start address (0 = reset)
		_send_command(oled_info, 3);		// Page end address
		_send_data(oled_info, (const uint8_t *) &clear_buffer, (SSD1306_LCD_WIDTH * 32 / 8) + 1);
		break;
	default:
		break;
	}
}

void oled_set_cursor(const oled_info_t *oled_info, uint8_t row, uint8_t col) {
	uint8_t _row;
	uint8_t _col;

	// row is 8 pixels tall; must set to byte sized row
	switch (oled_info->type) {
	case OLED_PANEL_128x64:
		if (row > (64 / OLED_FONT8x6_CHAR_H)) {
			_row = 0;
		} else {
			_row = row;
		}
		break;
	case OLED_PANEL_128x32:
		if (row > (32 / OLED_FONT8x6_CHAR_H)) {
			_row = 0;
		} else {
			_row = row;
		}
		break;
	default:
		_row = 0;
		break;
	}

	// _col is 1 pixel wide; can set to any pixel column
	if (col > SSD1306_LCD_WIDTH / OLED_FONT8x6_CHAR_W) {
		_col = 0;
	} else  {
		_col = col * OLED_FONT8x6_CHAR_W;
	}

	_send_command(oled_info, (uint8_t) SSD1306_CMD_SET_LOWCOLUMN | (_col & 0XF));
	_send_command(oled_info, (uint8_t) SSD1306_CMD_SET_HIGHCOLUMN | (_col >> 4));
	_send_command(oled_info, (uint8_t) SSD1306_CMD_SET_STARTPAGE | _row);
}

int oled_putc(const oled_info_t *oled_info, int c) {
	uint8_t i;
	uint8_t *base;

	if (c < 32 || c > 127) {
		i = (uint8_t) 0;
	} else {
		i = (uint8_t) c - 32;
	}

	base = oled_font + (uint8_t) (OLED_FONT8x6_CHAR_W + 1) * i;

	_send_data(oled_info, (const uint8_t*) base, (uint32_t) (OLED_FONT8x6_CHAR_W + 1));

	return c;
}

int oled_puts(const oled_info_t *oled_info, const char *s) {
	char c;
	int i = 0;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) oled_putc(oled_info, (int) c);
	}

	return i;
}

void oled_write(const oled_info_t *oled_info, const char *s, int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		(void) oled_putc(oled_info, (int) c);
	}
}

int oled_printf(const oled_info_t *oled_info, const char *format, ...) {
	int i;
	char buffer[(SSD1306_LCD_WIDTH / OLED_FONT8x6_CHAR_W) + 1] __attribute__((aligned(4)));

	va_list arp;

	va_start(arp, format);

	i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

	va_end(arp);

	oled_write(oled_info, buffer, i);

	return i;
}

void oled_clear_line(const oled_info_t *oled_info, const int line) {
	int i;

	oled_set_cursor(oled_info, (uint8_t) line, (uint8_t) 0);

	for (i = 0; i < (SSD1306_LCD_WIDTH / OLED_FONT8x6_CHAR_W); i++) {
		(void) oled_putc(oled_info, (int) ' ');
	}

	oled_set_cursor(oled_info, (uint8_t) line, (uint8_t) 0);

}

void oled_status(const oled_info_t *oled_info, const char *s) {

	switch (oled_info->type) {
	case OLED_PANEL_128x64:
		oled_clear_line(oled_info, 7);
		break;
	case OLED_PANEL_128x32:
		oled_clear_line(oled_info, 3);
		break;
	default:
		break;
	}

	oled_write(oled_info, s, (SSD1306_LCD_WIDTH / OLED_FONT8x6_CHAR_W));
}

const bool oled_start(oled_info_t *oled_info) {
	int i;

	if (oled_info->protocol == OLED_PROTOCOL_I2C) {
		i2c_begin();

		if (oled_info->slave_address == (uint8_t) 0) {
			oled_info->slave_address = OLED_I2C_SLAVE_ADDRESS_DEFAULT;
		}

		i2c_setup(oled_info);

		if (!i2c_is_connected(oled_info->slave_address)) {
			return false;
		}
	}
#if !defined(H3)
	else if (oled_info->protocol == OLED_PROTOCOL_SPI) {
		if (oled_info->speed_hz == (uint32_t) 0) {
			oled_info->speed_hz = (uint32_t) OLED_SPI_SPEED_DEFAULT_HZ;
		} else if (oled_info->speed_hz > (uint32_t) OLED_SPI_SPEED_MAX_HZ) {
			oled_info->speed_hz = (uint32_t) OLED_SPI_SPEED_MAX_HZ;
		}

		if (oled_info->chip_select >= OLED_SPI_CS2) {
			oled_info->chip_select = OLED_SPI_CS2;
			bcm2835_aux_spi_begin();
			oled_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(oled_info->speed_hz);
		} else {
			bcm2835_spi_begin();
			oled_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / oled_info->speed_hz);
		}
		bcm2835_gpio_fsel(OLED_RST, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_set(OLED_RST);
		bcm2835_gpio_fsel(OLED_DC, BCM2835_GPIO_FSEL_OUTP);

		if (oled_info->reset) {
			reset();
		}
	}
#endif
	else {
		return false;
	}

	switch (oled_info->type) {
	case OLED_PANEL_128x64:
		for (i = 0; i < (int) sizeof(oled_128x64_init); i++) {
			_send_command(oled_info, oled_128x64_init[i]);
		}
		break;
	case OLED_PANEL_128x32:
		for (i = 0; i < (int) sizeof(oled_128x32_init); i++) {
			_send_command(oled_info, oled_128x32_init[i]);
		}
		break;
	default:
		return false;
	}

	for (i = 0; i < (int) sizeof(clear_buffer); i++) {
		clear_buffer[i] = (uint8_t) 0;
	}

	clear_buffer[0] = (uint8_t) 0x40;

	oled_clear(oled_info);

	return true;
}

