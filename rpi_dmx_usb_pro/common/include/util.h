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

#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )

#define TO_HEX(i)	((i) < 10) ? (char)'0' + (char)(i) : (char)'A' + (char)((i) - 10)	///<

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

/**
 *
 * @param c
 * @return
 */
inline static bool is_digit(char c) {
	return (c >= (char)'0') && (c <= (char)'9');
}

/**
 *
 * @param c
 * @return
 */
inline static bool is_xdigit(char c) {
	return (is_digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

#endif /* UTIL_H_ */
