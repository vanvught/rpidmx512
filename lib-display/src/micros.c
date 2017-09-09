/**
 * @file micros.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "stdint.h"

#if defined(__linux__) || defined (__CYGWIN__)
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
uint32_t micros(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * (__time_t) 1000000) + tv.tv_usec;
}
#else
#if defined ( RPI1 )
#define BCM2835_PERI_BASE		0x20000000
#else
#define BCM2835_PERI_BASE		0x3F000000
#endif
#define BCM2835_ST_BASE			(BCM2835_PERI_BASE + 0x003000)
uint32_t micros(void) {
	return *(volatile uint32_t *) (BCM2835_ST_BASE + 0x04);
}
#endif
