/**
 * @file console_fb.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

// some RGB color definitions
typedef enum {
	CONSOLE_BLACK = 0x0000,		///<   0,   0,   0
	CONSOLE_BLUE = 0x001F,		///<   0,   0, 255
	CONSOLE_GREEN = 0x07E0,		///<   0, 255,   0
	CONSOLE_CYAN = 0x07FF,		///<   0, 255, 255
	CONSOLE_RED = 0xF800,		///< 255,   0,   0
	CONSOLE_YELLOW = 0xFFE0,	///< 255, 255,   0
	CONSOLE_WHITE = 0xFFFF,		///< 255, 255, 255
} _console_colors;

void console_clear();
void console_set_top_row(uint32_t);
void console_clear_line(uint32_t);
void console_set_cursor(uint32_t, uint32_t);
void console_save_cursor();
void console_restore_cursor();
void console_save_color(void);
void console_restore_color(void);
void console_puthex(uint8_t);
void console_set_fg_color(uint32_t);
void console_set_bg_color(uint32_t);
void console_set_fg_bg_color(uint16_t, uint16_t);
void console_puthex_fg_bg(uint8_t, uint16_t, uint16_t);
void console_putpct_fg_bg(uint8_t, uint16_t, uint16_t);
void console_put3dec_fg_bg(uint8_t, uint16_t, uint16_t);

#endif /* CONSOLE_FB_H_ */
