/**
 * @file console.h
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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

extern const uint32_t console_get_address(void);
extern const uint32_t console_get_pitch(void);
extern const uint32_t console_get_width(void);
extern const uint32_t console_get_height(void);
extern const uint32_t console_get_size(void);
extern const uint32_t console_get_depth(void);

extern int console_init(void);
extern int console_putc(const int);
extern int console_putc_inverted(const int);
extern void console_puts(const char *);
extern void console_puthex(const uint8_t);
extern void console_puthex_inverted(const uint8_t);
extern void console_newline(void);
extern void console_clear();
extern void console_set_cursor(const int, const int);
extern int console_draw_char(const int, const int, const int, const uint16_t, const uint16_t);
extern void console_set_fg_color(const uint16_t);
extern void console_set_bg_color(const uint16_t);
extern void console_clear_line(const int);

#define RGB(r, g, b) ((((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3))

// some RGB color definitions
#define CONSOLE_BLACK		0x0000	///<   0,   0,   0
#define CONSOLE_BLUE		0x001F	///<   0,   0, 255
#define CONSOLE_GREEN		0x07E0	///<   0, 255,   0
#define CONSOLE_CYAN		0x07FF	///<   0, 255, 255
#define CONSOLE_RED			0xF800	///< 255,   0,   0
#define CONSOLE_YELLOW		0xFFE0	///< 255, 255,   0
#define CONSOLE_WHITE		0xFFFF	///< 255, 255, 255

#define CONSOLE_OK		0			///< Call console_init() OK
#define CONSOLE_ERROR	-1			///< Call console_init() failed

#endif

