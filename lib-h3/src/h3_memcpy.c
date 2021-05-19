/**
 * @file h3_memcpy.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

//#include <stdio.h>

void* h3_memcpy(void *__restrict__ dest, void const *__restrict__ src, size_t n) {
//	assert(((uint32_t )((uint32_t* ) src) & 0x3) == 0);
//	assert(((uint32_t )((uint32_t* ) dest) & 0x3) == 0);

//	if (((uint32_t )((uint32_t* ) src) & 0x3) != 0) {
//		printf("src=%p\n", (void *)src);
//	}
//
//	if (((uint32_t )((uint32_t* ) dest) & 0x3) != 0) {
//		printf("dest=%p\n", (void *)dest);
//	}

	uint32_t *plDst = (uint32_t*) dest;
	uint32_t const *plSrc = (uint32_t const*) src;

	if ((((uint32_t) src & 0x3) == 0) && (((uint32_t) dest & 0x3) == 0)) {
		while (n >= 4) {
			*plDst++ = *plSrc++;
			n -= 4;
		}
	}

	uint8_t *pcDst = (uint8_t*) plDst;
	uint8_t const *pcSrc = (uint8_t const*) plSrc;

	while (n--) {
		*pcDst++ = *pcSrc++;
	}

	return dest;
}
