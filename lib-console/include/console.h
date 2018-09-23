/**
 * @file console.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdint.h>

#define RGB(r, g, b) ((((uint16_t)(r) & 0xF8) << 8) | (((uint16_t)(g) & 0xFC) << 3) | ((uint16_t)(b) >> 3))

#define CONSOLE_OK	0	///< Call console_init() OK

#if defined(H3) && !defined(CONSOLE_ILI9340)
 #include "h3/console.h"
#else
 #include "rpi/console.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int console_init(void);

extern void console_clear(void);

extern int console_putc(int);
extern int console_puts(const char *);

extern void console_set_fg_color(uint16_t);
extern void console_set_bg_color(uint16_t);
extern void console_set_fg_bg_color(uint16_t, uint16_t);

extern int console_status(uint16_t, const char *);
extern int console_error(const char *);

extern void console_puthex(uint8_t);
extern void console_puthex_fg_bg(uint8_t, uint16_t, uint16_t);

extern void console_newline(void);

extern void console_write(const char *, unsigned int);

extern void console_draw_pixel(uint16_t, uint16_t, uint16_t);

extern void console_clear_line(uint16_t);

extern uint16_t console_get_top_row(void);
extern void console_set_top_row(uint16_t);

extern void console_set_cursor(uint16_t, uint16_t);

extern void console_save_cursor(void);
extern void console_restore_cursor(void);

extern void console_save_color(void);
extern void console_restore_color(void);

extern uint16_t console_get_line_width(void);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H_ */
