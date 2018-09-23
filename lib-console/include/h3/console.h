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

#ifndef H3_CONSOLE_H_
#define H3_CONSOLE_H_

// ANSI colors
typedef enum {
	CONSOLE_BLACK = 0,
	CONSOLE_RED = 1,
	CONSOLE_GREEN = 2,
	CONSOLE_YELLOW = 3,
	CONSOLE_BLUE = 4,
	CONSOLE_MAGENTA = 5
,	CONSOLE_CYAN = 6,
	CONSOLE_WHITE = 7,
	CONSOLE_DEFAULT = 9
} _console_colors;

#ifdef __cplusplus
extern "C" {
#endif

// The following functions are not supported with debug UART
inline static void console_set_top_row(uint16_t __d) {}
inline static void console_clear_line(uint16_t __d) {}
inline static void console_set_cursor(uint16_t __d, uint16_t __e) {}
inline static void console_save_cursor(void) {}
inline static void console_restore_cursor(void) {}

#ifdef __cplusplus
}
#endif

#endif /* H3_CONSOLE_H_ */
