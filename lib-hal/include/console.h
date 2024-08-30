/**
 * @file console.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef __cplusplus
# include <cstdint>
# define RGB(r, g, b) static_cast<uint16_t>(((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | (((b) & 0xFF))))
#else
# include <stdint.h>
# define RGB(r, g, b) ((((uint32_t)(r) & 0xFF) << 16) | (((uint32_t)(g) & 0xFF) << 8) | (((uint32_t)(b) & 0xFF)))
#endif

#if defined (CONSOLE_NULL) || !defined (CONSOLE_FB)
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
#endif

#if defined (CONSOLE_FB)
# if defined (H3)
#  include "h3/console_fb.h"
# else
#  include "rpi/console_fb.h"
# endif
#elif defined (CONSOLE_NULL)
inline void console_init(void) {}
inline void console_putc([[maybe_unused]] int i) {}
inline void console_puts([[maybe_unused]] const char *p) {}
inline void console_write([[maybe_unused]] const char *p, [[maybe_unused]] unsigned int i) {}
inline void console_status([[maybe_unused]] uint32_t i, [[maybe_unused]] const char *p) {}
extern "C" inline void console_error([[maybe_unused]] const char *p) {}
#else
#ifdef __cplusplus
extern "C" {
#endif

extern void console_set_fg_color(uint16_t);
extern void console_set_bg_color(uint16_t);

#ifdef __cplusplus
}
#endif
#endif

#if !defined (CONSOLE_NULL)
extern void console_init();
#ifdef __cplusplus
extern "C" {
#endif
extern void console_putc(int);
extern void console_puts(const char*);
extern void console_write(const char*, unsigned int);
extern void console_status(uint32_t, const char *);
extern void console_error(const char*);
#ifdef __cplusplus
}
#endif
#endif

#endif /* CONSOLE_H_ */
