/**
 * @file bw_spi_lcd.c
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
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "bob.h"

#include "bw.h"
#include "bw_spi.h"
#include "bw_spi_lcd.h"

#define BW_LCD_SPI_BYTE_WAIT_US		12

#if defined (BARE_METAL)
static uint32_t spi_write_us = (uint32_t) 0;
#endif

inline static void spi_write(const char *buffer, uint32_t size) {
#if defined (BARE_METAL)
 #if defined (H3)
	const uint32_t elapsed = h3_hs_timer_lo_us() - spi_write_us;
 #else
	const uint32_t elapsed = BCM2835_ST->CLO - spi_write_us;
 #endif

	if (elapsed < BW_LCD_SPI_BYTE_WAIT_US) {
		udelay(BW_LCD_SPI_BYTE_WAIT_US - elapsed);
	}
#else
	udelay(BW_LCD_SPI_BYTE_WAIT_US);
#endif
	(void) bcm2835_spi_writenb(buffer, size);
#if defined (BARE_METAL)
 #if defined (H3)
	spi_write_us = h3_hs_timer_lo_us();
 #else
	spi_write_us = BCM2835_ST->CLO;
 #endif
#endif
}

bool bw_spi_lcd_start(device_info_t *device_info) {

	if (device_info->slave_address == (uint8_t) 0) {
		device_info->slave_address = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	}

	if (device_info->speed_hz == (uint32_t) 0) {
		device_info->speed_hz = (uint32_t) BW_LCD_SPI_SPEED_DEFAULT_HZ;
	} else if (device_info->speed_hz > (uint32_t) BW_LCD_SPI_SPEED_MAX_HZ) {
		device_info->speed_hz = (uint32_t) BW_LCD_SPI_SPEED_MAX_HZ;
	}

	if (device_info->chip_select >= SPI_CS2) {
		device_info->chip_select = SPI_CS2;
		bcm2835_aux_spi_begin();
		device_info->internal.clk_div = bcm2835_aux_spi_CalcClockDivider(device_info->speed_hz);
	} else {
		bcm2835_spi_begin();
		device_info->internal.clk_div = (uint16_t)((uint32_t) BCM2835_CORE_CLK_HZ / device_info->speed_hz);
	}

	char id[BW_ID_STRING_LENGTH+1];
	bw_spi_read_id(device_info, id);

	if (memcmp(id, "spi_lcd", 7) == 0) {
#if defined (BARE_METAL)
 #if defined (H3)
		spi_write_us = h3_hs_timer_lo_us();
 #else
		spi_write_us = BCM2835_ST->CLO;
 #endif
#endif
		return true;
	}

	return false;
}

void bw_spi_lcd_set_cursor(const device_info_t *device_info, uint8_t line, uint8_t pos) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_MOVE_CURSOR, (char) 0x00 };

	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) (((line & 0x03) << 5) | (pos & 0x1f));

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

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

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(data, length + 2);
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(data, length + 2);
	}
}

void bw_spi_lcd_text_line_1(const device_info_t *device_info, const char *text, uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 0, 0);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_2(const device_info_t *device_info, const char *text,	uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 1, 0);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_3(const device_info_t *device_info, const char *text,	uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 2, 0);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_text_line_4(const device_info_t *device_info, const char *text,	uint8_t length) {
	bw_spi_lcd_set_cursor(device_info, 3, 0);
	bw_spi_lcd_text(device_info, text, length);
}

void bw_spi_lcd_cls(const device_info_t *device_info) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_CLEAR_SCREEN, (char) ' ' };

	cmd[0] = (char) device_info->slave_address;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

void bw_spi_lcd_set_contrast(const device_info_t *device_info, uint8_t value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_SET_CONTRAST, (char) 0x00 };

	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) value;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

void bw_spi_lcd_set_backlight(const device_info_t *device_info,	uint8_t value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_SET_BACKLIGHT, (char) 0x00 };

	cmd[0] = (char) device_info->slave_address;
	cmd[2] = (char) value;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

void bw_spi_lcd_reinit(const device_info_t *device_info) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_WRITE_REINIT_LCD, (char) ' ' };

	cmd[0] = (char) device_info->slave_address;

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_aux_spi_writenb(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}
}

void bw_spi_lcd_get_backlight(const device_info_t *device_info, uint8_t *value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_READ_CURRENT_BACKLIGHT, (char) 0x00 };

	cmd[0] = (char) (device_info->slave_address | 1);

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(32000));
		bcm2835_aux_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}

	*value = (uint8_t)cmd[2];
}

void bw_spi_lcd_get_contrast(const device_info_t *device_info, uint8_t *value) {
	char cmd[] = { (char) 0x00, (char) BW_PORT_READ_CURRENT_CONTRAST, (char) 0x00 };

	cmd[0] = (char) (device_info->slave_address | 1);

	if (device_info->chip_select == SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(32000));
		bcm2835_aux_spi_transfern(cmd, sizeof(cmd) / sizeof(cmd[0]));
	} else {
		bcm2835_spi_setClockDivider(device_info->internal.clk_div);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		spi_write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	}

	*value = (uint8_t) cmd[2];
}
