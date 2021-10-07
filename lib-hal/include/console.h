/**
 * @file console.h
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#define CONSOLE_OK	0	///< Call console_init() OK

#ifdef __cplusplus
# define RGB(r, g, b) static_cast<uint16_t>(((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | (((b) & 0xFF))))
#else
# define RGB(r, g, b) ((((uint32_t)(r) & 0xFF) << 16) | (((uint32_t)(g) & 0xFF) << 8) | (((uint32_t)(b) & 0xFF)))
#endif

#if defined (CONSOLE_FB)
# if defined (H3)
#  include "h3/console_fb.h"
# else
#  include "rpi/console_fb.h"
# endif
#else
# include "console_uart0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int console_init(void);

extern void console_putc(int);
extern void console_puts(const char*);
extern void console_write(const char*, unsigned int);

extern void console_status(uint32_t, const char *);
extern void console_error(const char*);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H_ */
