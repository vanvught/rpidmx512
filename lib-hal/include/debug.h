/**
 * @file debug.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DEBUG_H_
#define DEBUG_H_

#ifndef NDEBUG
 #include <stdio.h>
 #define DEBUG_ENTRY	printf("--> %s:%s:%d\n", __FILE__, __func__, __LINE__);
 #define DEBUG_EXIT		printf("<-- %s:%s:%d\n", __FILE__, __func__, __LINE__);
#else
 #define DEBUG_ENTRY	((void)0);
 #define DEBUG_EXIT		((void)0);
#endif

#ifdef NDEBUG
 #define DEBUG_PRINTF(FORMAT, ...)	((void)0)
 #define DEBUG_PUTS(MSG)			((void)0)
 #define debug_dump(x,y)			((void)0)
 #define debug_print_bits(x)		((void)0)
#else
 #include <stdint.h>
 void debug_dump(const void *, uint32_t);
 void debug_print_bits(uint32_t);
 #if defined (__linux__) || defined (__CYGWIN__)
  #define DEBUG_PRINTF(FORMAT, ...) \
    fprintf(stderr, "%s() %s, line %i: " FORMAT "\n", __func__, __FILE__, __LINE__, __VA_ARGS__)
 #else
  #define DEBUG_PRINTF(FORMAT, ...) \
    printf("%s() %s, line %i: " FORMAT "\n", __func__, __FILE__, __LINE__, __VA_ARGS__)
 #endif
 #define DEBUG_PUTS(MSG) DEBUG_PRINTF("%s", MSG)
#endif

#endif /* DEBUG_H_ */
