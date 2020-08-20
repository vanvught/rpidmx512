/**
 * @file console_fb.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef CONSOLE_FB_H_
#define CONSOLE_FB_H_

#if !defined ORANGE_PI_ONE
 #error Support for Orange Pi One only
#endif

#include <stdint.h>

// some RGB color definitions
typedef enum {
	CONSOLE_BLACK = 0x00000000,		///<   0,   0,   0
	CONSOLE_BLUE = 0x000000FF,		///<   0,   0, 255
	CONSOLE_GREEN = 0x0000FF00,		///<   0, 255,   0
	CONSOLE_CYAN = 0x0000FFFF,		///<   0, 255, 255
	CONSOLE_RED = 0x00FF0000,		///< 255,   0,   0
	CONSOLE_YELLOW = 0x00FFFF00,	///< 255, 255,   0
	CONSOLE_WHITE = 0x00FFFFFF		///< 255, 255, 255
} _console_colors;

#ifdef __cplusplus
extern "C" {
#endif

extern void console_set_fg_color(uint32_t);
extern void console_set_bg_color(uint32_t);
extern void console_set_fg_bg_color(uint32_t, uint32_t);

extern void console_puthex_fg_bg(uint8_t, uint32_t, uint32_t);
extern void console_putpct_fg_bg(uint8_t, uint32_t, uint32_t);
extern void console_put3dec_fg_bg(uint8_t, uint32_t, uint32_t);

extern void console_status(uint32_t, const char *);

extern void console_clear_top_row(void);

extern void console_putpixel(uint32_t x, uint32_t y, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_FB_H_ */
