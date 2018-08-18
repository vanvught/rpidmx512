/**
 * @file debug_dump.c
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

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

#define CHARS_PER_LINE 16

void debug_dump(void *packet, uint16_t len) {
	uint16_t chars = 0;
	uint16_t chars_this_line = 0;
	uint8_t *p = (uint8_t *) packet;

	do {
		chars_this_line = 0;

		printf("%04x ", chars);

		uint8_t *q = p;

		while ((chars_this_line < CHARS_PER_LINE) && (chars < len)) {
			if (chars_this_line % 8 == 0) {
				printf(" ");
			}

			printf("%02x ", *p);

			chars_this_line++;
			chars++;
			p++;
		}

		uint16_t chars_dot_line = chars_this_line;

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

		printf("\n");

	} while (chars < len);

}
