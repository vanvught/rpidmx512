/**
 * @file i2c_begin.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>
#include <stdbool.h>

#if defined(H3)
#else
 #include "bcm2835.h"
#endif

#if defined(__linux__)
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/types.h>
 static bool _begin = false;
#elif defined(H3)
 #include "h3_i2c.h"
#else
 #include "bcm2835_i2c.h"
#endif

#if defined(H3)
 #define FUNC_PREFIX(x) h3_##x
#else
 #define FUNC_PREFIX(x) bcm2835_##x
#endif

bool i2c_begin(void) {
#if defined(__linux__)
	if (_begin) {
		return true;
	}

	if (getuid() != 0) {
		fprintf(stderr, "Error: Not started with 'root'\n");
		return false;
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		return false;
	} else {
		_begin = true;
	}

	if (bcm2835_i2c_begin() != 1) {
		fprintf(stderr, "bcm2835_i2c_begin() failed\n");
	}
#else
	FUNC_PREFIX(i2c_begin());
#endif

	return true;
}
