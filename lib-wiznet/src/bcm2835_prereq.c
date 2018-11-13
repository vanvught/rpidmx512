/**
 * @file w5x00_prereq.c
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

#include <stdio.h>
#if defined (__linux__)
 #include <unistd.h>
#endif

#if defined (__linux__)  || defined(__circle__)
	#include "bcm2835.h"
#endif

#include "w5x00.h"

#include "debug.h"

int bcm2835_prereq(void) {
	DEBUG1_ENTRY
#if defined (__linux__)
	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		DEBUG1_EXIT
		return W5X00_INIT_NOT_ROOT;
	}
#endif
#if defined (__linux__)  || defined(__circle__)
	if (bcm2835_init() == 0) {
		printf("Not able to init the bmc2835 library\n");
		DEBUG1_EXIT
		return W5X00_INIT_LIB_INIT_FAILED;
	}
#endif
	DEBUG1_EXIT
	return 0;
}
