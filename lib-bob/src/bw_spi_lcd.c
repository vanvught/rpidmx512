/**
 * @file bw_spi_lcd.c
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

#include "bcm2835_spi.h"
#include "bcm2835_aux_spi.h"

#include "bw.h"
#include "bw_lcd.h"
#include "bw_spi_lcd.h"

#include "device_info.h"

#define BW_LCD_SPI_BYTE_WAIT_US			10		///<

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @return
 */
void bw_spi_lcd_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) BW_LCD_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) BW_LCD_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) BW_LCD_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}
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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
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

	if (length > BW_LCD_MAX_CHARACTERS) {
		length = BW_LCD_MAX_CHARACTERS;
	}

	for (i = 0; i < length; i++) {
		data[i + 2] = text[i];
	}

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(data, length + 2);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(data, length + 2);
	}
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 * @param text
 * @param length
 */
void bw_spi_lcd_text_line_1(const device_info_t *device_info, const char *text, const uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 0, 0);
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
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
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
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
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
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
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

/**
 * @ingroup SPI-LCD
 *
 * @param device_info
 */
void bw_spi_lcd_reinit(const device_info_t *device_info) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_REINIT_LCD, (char) ' ' };

	cmd[0] = (char) device_info->slave_address;

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(32000));
		bcm2835_aux_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}

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

	if (device_info->chip_select == (uint8_t) 2) {
		bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(32000));
		bcm2835_aux_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}

	*value = (uint8_t)cmd[2];
}
