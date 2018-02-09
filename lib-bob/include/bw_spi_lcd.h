/**
 * @file bw_spi_lcd.h
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

#ifndef BW_SPI_LCD_H_
#define BW_SPI_LCD_H_

#include <stdint.h>
#include <stdbool.h>

#include "device_info.h"

#define BW_LCD_SPI_SPEED_MAX_HZ			90000	///< 90 KHz
#define BW_LCD_SPI_SPEED_DEFAULT_HZ		90000	///< 90 kHz

#ifdef __cplusplus
extern "C" {
#endif

extern bool bw_spi_lcd_start (device_info_t *);

extern void bw_spi_lcd_reinit(const device_info_t *);

extern void bw_spi_lcd_cls(const device_info_t *);

extern void bw_spi_lcd_set_cursor(const device_info_t *, uint8_t, uint8_t);

extern void bw_spi_lcd_text(const device_info_t *, const char *, uint8_t);
extern void bw_spi_lcd_text_line_1(const device_info_t *, const char *, uint8_t);
extern void bw_spi_lcd_text_line_2(const device_info_t *, const char *, uint8_t);

extern void bw_spi_lcd_get_contrast(const device_info_t *, uint8_t *);
extern void bw_spi_lcd_set_contrast(const device_info_t *, uint8_t);

extern void bw_spi_lcd_get_backlight(const device_info_t *, uint8_t *);
extern void bw_spi_lcd_set_backlight(const device_info_t *, uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* BW_SPI_LCD_H_ */
