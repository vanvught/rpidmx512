/**
 * @file oled.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef OLED_H_
#define OLED_H_

#include <stdbool.h>
#include <stdarg.h>

#define OLED_I2C_SLAVE_ADDRESS_DEFAULT	0x3C	///<

#define OLED_SPI_SPEED_MAX_HZ			8000000	///< 8 MHz
#define OLED_SPI_SPEED_DEFAULT_HZ		1000000	///< 1 MHz

typedef enum oled_panel {
	OLED_PANEL_128x64, 	///< OLED_PANEL_128x64
	OLED_PANEL_128x32 	///< OLED_PANEL_128x32
} oled_panel_t;

typedef enum oled_protocol {
	OLED_PROTOCOL_I2C,
	OLED_PROTOCOL_SPI
} oled_protocol_t;

typedef enum oled_spi_cs {
	OLED_SPI_CS0,
	OLED_SPI_CS1,
	OLED_SPI_CS2
} oled_spi_cs_t;

typedef struct _oled_info {
	uint8_t slave_address;
	oled_panel_t type;
	bool reset;
	oled_protocol_t protocol;
	oled_spi_cs_t chip_select;
	uint32_t speed_hz;
	struct _oled_info_internal {
		uint16_t clk_div;
	} internal;
} oled_info_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const bool oled_start(oled_info_t *);

extern void oled_clear(const oled_info_t *);

extern void oled_set_cursor(const oled_info_t *, const uint8_t, const uint8_t);

extern int oled_putc(const oled_info_t *, const int);
extern int oled_puts(const oled_info_t *, const char *);

extern void oled_write(const oled_info_t *, const char *, int);

extern int oled_printf(const oled_info_t *, const char *, ...);

extern void oled_clear_line(const oled_info_t *, const int);

extern void oled_status(const oled_info_t *, const char *);

#ifdef __cplusplus
}
#endif

#endif /* OLED_H_ */
