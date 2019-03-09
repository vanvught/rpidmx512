/**
 * @file ili9340.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ILI9340_H_
#define ILI9340_H_

#include <stdint.h>

#define ILI9340_OK		0

typedef enum {
	ILI9340_WIDTH = 320,
	ILI9340_HEIGHT = 240
} _ili9340_size;

// some RGB color definitions
typedef enum {
	ILI9340_BLACK = 0x0000,		///<   0,   0,   0
	ILI9340_BLUE = 0x001F,		///<   0,   0, 255
	ILI9340_GREEN = 0x07E0,		///<   0, 255,   0
	ILI9340_CYAN = 0x07FF,		///<   0, 255, 255
	ILI9340_RED = 0xF800,		///< 255,   0,   0
	ILI9340_YELLOW = 0xFFE0,	///< 255, 255,   0
	ILI9340_WHITE = 0xFFFF,		///< 255, 255, 255
} _ili9340_colors;

#define RGB(r, g, b) ((((uint16_t)(r) & 0xF8) << 8) | (((uint16_t)(g) & 0xFC) << 3) | ((uint16_t)(b) >> 3))

#ifdef __cplusplus
extern "C" {
#endif

extern int ili9340_init(void);
extern void ili9340_clear(void);

extern int ili9340_putc(int);
extern int ili9340_puts(const char *);
extern void ili9340_write(const char *, unsigned int);
extern int ili9340_error(const char *);

extern void ili9340_set_top_row(uint16_t);

extern void ili9340_clear_line(uint16_t);
extern void ili9340_newline(void);

extern void ili9340_set_cursor(uint16_t, uint16_t);
extern void ili9340_save_cursor(void);
extern void ili9340_restore_cursor(void);

extern void ili9340_save_color(void);
extern void ili9340_restore_color(void);

extern void ili9340_draw_pixel(uint16_t, uint16_t, uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* ILI9340_H_ */
