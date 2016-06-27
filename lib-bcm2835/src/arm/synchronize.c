/**
 * @file synchronize.c
 *
 */
/* This code is inspired by:
 *
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 * https://github.com/rsta2/circle/blob/master/lib/synchronize.cpp
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

#if defined (RPI2) && defined (RPI3)
#error You cannot have defined both RPI2 and RPI3
#endif

#if defined (RPI2) || defined (RPI3)
#include <stdint.h>

#define SETWAY_LEVEL_SHIFT			1

#define L1_DATA_CACHE_SETS			128
#define L1_DATA_CACHE_WAYS			4
#define L1_SETWAY_WAY_SHIFT			30	// 32-Log2(L1_DATA_CACHE_WAYS)
#define L1_DATA_CACHE_LINE_LENGTH	64
#define L1_SETWAY_SET_SHIFT			6	// Log2(L1_DATA_CACHE_LINE_LENGTH)

#if defined (RPI2)
#define L2_CACHE_SETS				1024
#define L2_CACHE_WAYS				8
#define L2_SETWAY_WAY_SHIFT			29	// 32-Log2(L2_CACHE_WAYS)
#elif defined (RPI3)
#define L2_CACHE_SETS				512
#define L2_CACHE_WAYS				16
#define L2_SETWAY_WAY_SHIFT			28	// 32-Log2(L2_CACHE_WAYS)
#endif
#define L2_CACHE_LINE_LENGTH		64
#define L2_SETWAY_SET_SHIFT			6	// Log2(L2_CACHE_LINE_LENGTH)

#define DATA_CACHE_LINE_LENGTH_MIN	64	// min(L1_DATA_CACHE_LINE_LENGTH, L2_CACHE_LINE_LENGTH)


void invalidate_data_cache(void) {
	register unsigned nSet;
	register unsigned nWay;

	// invalidate L1 data cache
	for (nSet = 0; nSet < L1_DATA_CACHE_SETS; nSet++) {
		for (nWay = 0; nWay < L1_DATA_CACHE_WAYS; nWay++) {
			register uint32_t nSetWayLevel = nWay << L1_SETWAY_WAY_SHIFT
					| nSet << L1_SETWAY_SET_SHIFT | 0 << SETWAY_LEVEL_SHIFT;

			asm volatile ("mcr p15, 0, %0, c7, c6,  2" : : "r" (nSetWayLevel) : "memory"); // DCISW
		}
	}

	// invalidate L2 unified cache
	for (nSet = 0; nSet < L2_CACHE_SETS; nSet++) {
		for (nWay = 0; nWay < L2_CACHE_WAYS; nWay++) {
			register uint32_t nSetWayLevel = nWay << L2_SETWAY_WAY_SHIFT
					| nSet << L2_SETWAY_SET_SHIFT | 1 << SETWAY_LEVEL_SHIFT;

			asm volatile ("mcr p15, 0, %0, c7, c6,  2" : : "r" (nSetWayLevel) : "memory"); // DCISW
		}
	}
}

void clean_data_cache(void) {
	register unsigned nSet;
	register unsigned nWay;

	// clean L1 data cache
	for (nSet = 0; nSet < L1_DATA_CACHE_SETS; nSet++) {
		for (nWay = 0; nWay < L1_DATA_CACHE_WAYS; nWay++) {
			register uint32_t nSetWayLevel =   nWay << L1_SETWAY_WAY_SHIFT
						    | nSet << L1_SETWAY_SET_SHIFT
						    | 0 << SETWAY_LEVEL_SHIFT;

			asm volatile ("mcr p15, 0, %0, c7, c10,  2" : : "r" (nSetWayLevel) : "memory");	// DCCSW
		}
	}

	// clean L2 unified cache
	for (nSet = 0; nSet < L2_CACHE_SETS; nSet++) {
		for (nWay = 0; nWay < L2_CACHE_WAYS; nWay++) {
			register uint32_t nSetWayLevel =   nWay << L2_SETWAY_WAY_SHIFT
						    | nSet << L2_SETWAY_SET_SHIFT
						    | 1 << SETWAY_LEVEL_SHIFT;

			asm volatile ("mcr p15, 0, %0, c7, c10,  2" : : "r" (nSetWayLevel) : "memory");	// DCCSW
		}
	}
}

void invalidate_data_cache_l1_only(void) {
	register unsigned nSet;
	register unsigned nWay;

	// invalidate L1 data cache
	for (nSet = 0; nSet < L1_DATA_CACHE_SETS; nSet++) {
		for (nWay = 0; nWay < L1_DATA_CACHE_WAYS; nWay++) {
			register uint32_t nSetWayLevel = nWay << L1_SETWAY_WAY_SHIFT
					| nSet << L1_SETWAY_SET_SHIFT | 0 << SETWAY_LEVEL_SHIFT;

			asm volatile ("mcr p15, 0, %0, c7, c6,  2" : : "r" (nSetWayLevel) : "memory"); // DCISW
		}
	}
}
#endif
