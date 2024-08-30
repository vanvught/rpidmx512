/**
 * @file log.c
 *
 */
/*
 * Based on http://www.flipcode.com/archives/Fast_log_Function.shtml
 * and https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c
 *
 * Reference https://www.doc.ic.ac.uk/~eedwards/compsys/float/nan.html
 * and http://steve.hollasch.net/cgindex/coding/ieeefloat.html
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

typedef union {
	float number;
	int32_t bits;
} float2bits;

/* Natural log of 2 */
#ifndef _M_LN2
#define _M_LN2        0.693147180559945309417f
#endif

/**
 * On success, the function return the base 2 logarithm of x.
 * maximum error ±0.00493976
 *
 * If x is 1, the result is +0.
 * If x is 0, the result is -infinity
 * If x is negative a NaN (not a number) is returned.
 */
float log2f(float x) {
	float2bits m;

	m.number = x;

	if (x == 0) {
		m.bits = (int32_t) 0xFF800000;	// -inf
		return m.number;
	} else if (x == 1) {
		return (float) 0;
	} else if (x < 0) {
		m.bits = (int32_t) 0x7F800001;	// nan
		return m.number;
	}

	register float log2 = (float)(((m.bits >> 23) & 0x00FF) - 128);

	m.bits &= ~(255 << 23);
	m.bits += (127 << 23);

	log2 += ((-0.34484843f) * m.number + 2.02466578f) * m.number - 0.67487759f;

	return log2;
}

float logf(float v) {
	return log2f(v) * _M_LN2;
}
