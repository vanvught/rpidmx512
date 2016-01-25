/**
 * @file util.h
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

#ifndef UTIL_H_
#define UTIL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )								///<

#define TO_HEX(i)		((i) < 10) ? (char)'0' + (char)(i) : (char)'A' + (char)((i) - 10)	///<

#ifndef MAX
#define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#if 0
/**
 * @ingroup util
 *
 * @param c
 * @return
 */
inline static bool _isdigit(char c) {
	return (c >= (char)'0') && (c <= (char)'9');
}

/**
 * @ingroup util
 *
 * @param c
 * @return
 */
inline static bool _isxdigit(char c) {
	return (_isdigit(c) || (c >= (char)'A' && c <= (char)'F') || (c >= (char)'a' && c <= (char)'f'));
}

/**
 * @ingroup util
 *
 * @param c
 * @return
 */
inline static bool _isprint(char c) {
	return ((c >= (char) 32 && c <= (char) 127) ? true : false);
}
#endif

/**
 * @ingroup util
 *
 * @param s1
 * @param s2
 * @param n
 * @return
 */
inline static int _memcmp(const void *s1, const void *s2, size_t n) {
	unsigned char u1, u2;
	unsigned char *t1, *t2;

	t1 = (unsigned char *)s1;
	t2 = (unsigned char *)s2;
	for (; n-- != (size_t)0; t1++, t2++) {
		u1 = *t1;
		u2 = *t2;
		if (u1 != u2) {
			return (int) (u1 - u2);
		}
	}

	return 0;
}

/**
 * @ingroup util
 *
 * @param dest
 * @param src
 * @param n
 */
inline static void *_memcpy(void *dest, const void *src, size_t n) {
	char *dp = dest;
	const char *sp = src;

	while (n-- != (size_t) 0) {
		*dp++ = *sp++;
	}

	return dest;
}

/**
 * @ingroup util
 *
 * @param s
 * @return
 */
inline static size_t _strlen(const char *s) {
	const char *p = s;

	while (*s != (char)0) {
		++s;
	}

	return (size_t) (s - p);
}

#define GCC_VERSION (__GNUC__ * 10000 \
                   + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)

#if GCC_VERSION > 40899
// 4.9 supports assume_aligned, 4.8 does not. 
#define ASSUME_ALIGNED  __attribute__((assume_aligned(4)))
#else
#define ASSUME_ALIGNED  
#endif

#define ALIGNED  		__attribute__((aligned(4)))

#endif /* UTIL_H_ */
