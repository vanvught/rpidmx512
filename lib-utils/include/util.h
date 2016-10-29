/**
 * @file util.h
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stddef.h>
#include <stdint.h>

#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )								///<

#define TO_HEX(i)		((i) < 10) ? (char)'0' + (char)(i) : (char)'A' + (char)((i) - 10)	///<

#ifndef MAX
#  define MAX(a,b)		(((a) > (b)) ? (a) : (b))											///<
#  define MIN(a,b)		(((a) < (b)) ? (a) : (b))											///<
#endif

#ifndef SWAP_UINT16
#  define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#endif
#ifndef SWAP_UINT32
#  define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#endif

#define isdigit(c)		((c) >= '0' && (c) <= '9' ? 1 : 0)									///<
#define isxdigit(c)		(isdigit(c) || ((unsigned) (c) | 32) - 'a' < 6)						///<
#define isprint(c)		(((c) >= ' ' && (c) <= '~') ? 1 : 0)								///<
#define isupper(c)		((c) >= 'A' && (c) <= 'Z')											///<
#define islower(c)		((c) >= 'a' && (c) <= 'z')											///<
#define isalpha(c)		(isupper(c) || islower(c))											///<
#define tolower(c)		(isupper(c) ? ((c) + 32) : (c))										///<
#define toupper(c)		(islower(c) ? ((c) - 32) : (c))										///<

#define memcpy			_memcpy
#define memcmp			_memcmp
#define memset			_memset
#define strlen			_strlen
#define strncpy			_strncpy
#define strcmp			_strcmp

#ifdef __cplusplus
extern "C" {
#endif

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

inline static void *_memcpy(void *dest, const void *src, size_t n) {
	char *dp = (char *)dest;
	const char *sp = (const char *)src;

	while (n-- != (size_t) 0) {
		*dp++ = *sp++;
	}

	return dest;
}

inline static void *_memset (void *dest, int c, size_t n) {
	char *dp = (char *)dest;

	while (n-- != (size_t) 0) {
		*dp++ = (char) c;
	}

	return dest;
}

inline static size_t _strlen(const char *s) {
	const char *p = s;

	while (*s != (char)0) {
		++s;
	}

	return (size_t) (s - p);
}

inline static char *_strncpy(char *s1, const char *s2, size_t n)
 {
	char *s = s1;

	while (n > 0 && *s2 != '\0') {
		*s++ = *s2++;
		--n;
	}

	while (n > 0) {
		*s++ = '\0';
		--n;
	}
	return s1;
}

inline static int _strcmp(const char *s1, const char *s2) {
	char *p1 = (char *) s1;
	char *p2 = (char *) s2;

	for (; *p1 == *p2; p1++, p2++) {
		if (*p1 == '\0') {
			return 0;
		}
	}
	return (int) ((*(unsigned char *) p1 < *(unsigned char *) p2) ? -1 : +1);
}

extern const uint32_t hex_uint32(const char *);

#ifdef __cplusplus
}
#endif

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
