/**
 * @file debug_dump.cpp
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

#include <cstdio>
#include <cstdint>
#include <ctype.h>

#if defined (H3)
extern "C" int uart0_printf(const char* fmt, ...);
# define printf uart0_printf
#endif

static constexpr uint32_t CHARS_PER_LINE = 16;

void debug_dump(const void *pData, uint32_t nSize) {
	uint32_t chars = 0;
	const auto *p = reinterpret_cast<const uint8_t *>(pData);

	printf("%p:%d\n", pData, nSize);

	do {
		uint32_t chars_this_line = 0;

		printf("%04x ", chars);

		const auto *q = p;

		while ((chars_this_line < CHARS_PER_LINE) && (chars < nSize)) {
			if (chars_this_line % 8 == 0) {
				printf(" ");
			}

			printf("%02x ", *p);

			chars_this_line++;
			chars++;
			p++;
		}

		auto chars_dot_line = chars_this_line;

		for (; chars_this_line < CHARS_PER_LINE; chars_this_line++) {
			if (chars_this_line % 8 == 0) {
				printf(" ");
			}
			printf("   ");

		}

		chars_this_line = 0;

		while (chars_this_line < chars_dot_line) {
			if (chars_this_line % 8 == 0) {
				printf(" ");
			}

			int ch = *q;
			if (isprint(ch)) {
				printf("%c", ch);
			} else {
				printf(".");
			}

			chars_this_line++;
			q++;
		}

		puts("");

	} while (chars < nSize);

}
