/**
 * @file console.h
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

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdint.h>

#define CONSOLE_OK	0	///< Call console_init() OK

#if !defined(CONSOLE_FB)
 #define RGB(r, g, b) ((((uint16_t)(r) & 0xF8) << 8) | (((uint16_t)(g) & 0xFC) << 3) | ((uint16_t)(b) >> 3))
#endif

#if defined(H3)
 #if defined(CONSOLE_FB)
  #include "h3/console_fb.h"
 #elif defined(CONSOLE_NONE)
	typedef enum {
		CONSOLE_BLACK,
		CONSOLE_BLUE,
		CONSOLE_GREEN,
		CONSOLE_CYAN,
		CONSOLE_RED,
		CONSOLE_YELLOW,
		CONSOLE_WHITE
	} _console_colors;
 #else
  #include "h3/console_uart0.h"
 #endif
#else
 #include "rpi/console_fb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONSOLE_NONE)
	inline static int console_init(void) {return CONSOLE_OK;}
	inline static void console_set_top_row(uint16_t _p1) {}
	inline static void console_set_fg_color(uint16_t _p) {}
	inline static void console_set_cursor(uint16_t _p1, uint16_t _p2) {}
	inline static void console_save_cursor(void) {}
    inline static void console_restore_cursor(void) {}
	inline static void console_status(uint16_t _p1, const char *_p2) {}
	inline static void console_puthex(uint8_t _p1) {}
	inline static void console_puthex_fg_bg(uint8_t _p1, uint16_t _p2, uint16_t _p3) {}
    inline static void console_putpct_fg_bg(uint8_t _p1, uint16_t _p2, uint16_t _p3) {}
    inline static void console_put3dec_fg_bg(uint8_t _p1, uint16_t _p2, uint16_t _p3) {}
	inline static void console_clear_line(uint16_t _p1) {}
	inline static void console_newline(void) {}
	inline static int console_putc(int _p1) {return 0;}
	inline static int console_puts(const char *_p1) {return 0;}
	inline static int console_error(const char *_p1)  {return 0;}
#else
	extern int console_init(void);
	extern void console_clear(void);

	extern void console_set_top_row(uint16_t);

	extern int console_putc(int);
	extern int console_puts(const char *);

	extern int console_error(const char *);

	extern void console_puthex(uint8_t);

	extern void console_newline(void);

	extern void console_write(const char *, unsigned int);

	extern void console_clear_line(uint16_t);

	extern void console_set_cursor(uint16_t, uint16_t);

	extern void console_save_cursor(void);
	extern void console_restore_cursor(void);

	extern void console_save_color(void);
	extern void console_restore_color(void);

	extern uint16_t console_get_line_width(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H_ */
