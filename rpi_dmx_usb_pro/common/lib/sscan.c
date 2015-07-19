/**
 * @file sscan.c
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

#include <stdint.h>
#include <stdbool.h>

inline static bool is_digit(char c) {
	return (c >= (char)'0') && (c <= (char)'9');
}

int sscan_uint8_t(const char *buf, const char *name, uint8_t *value) {
	int i;
	int k = 0;

	for (i = 0; (name[i] != (char) 0) && (buf[i] != (char) '=')	&& (buf[i] != (char) 0); i++) {
		if (name[i] != buf[i]) {
			return 0;
		}
	}

	if (name[i] != (char)0) {
		return 0;
	}

	i++;

	while ((buf[i] != (char) 0) && (buf[i] != '\n')) {
		if (!is_digit(buf[i])) {
			return 1;
		}
		k = k * 10 + (int)buf[i] - (int)'0';
		i++;
	}

	if (k > (int)((uint8_t)~0)) {
		return 1;
	}

	*value = (uint8_t)k;

	return 2;
}

int sscan_char_p(const char *buf, const char *name, char *value, uint8_t *len) {
	int i;
	int k;

	for (i = 0; (name[i] != (char) 0) && (buf[i] != (char) '=')	&& (buf[i] != (char) 0); i++) {
		if (name[i] != buf[i]) {
			return 0;
		}
	}

	if (name[i] != (char)0) {
		return 0;
	}

	i++;

	if ((buf[i] == (char) 0) || (buf[i] == '\n')) {
		return 1;
	}

	k = 0;

	while ((buf[i] != (char) 0) && (buf[i] != (char) '\n') && (k < (int)*len)) {
		value[k++] = buf[i++];
	}

//	if ((buf[i] != (char) 0) && (buf[i] != '\n')) {
//		value[k] = (char)0;
//		return 1;
//	}

	*len = (uint8_t)k;

	return 2;
}
