/**
 * @file h3_cpu.h
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

#ifndef H3_CPU_H_
#define H3_CPU_H_

#include <stdint.h>

#define H3_CPU_COUNT	4

typedef enum h3_cpu {
	H3_CPU0 = 0,
	H3_CPU1,
	H3_CPU2,
	H3_CPU3
} h3_cpu_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_cpu_on(h3_cpu_t);
extern void h3_cpu_off(h3_cpu_t);

extern void h3_cpu_set_clock(uint64_t);
extern uint64_t h3_cpu_get_clock(void);

#ifdef __cplusplus
}
#endif


#endif /* H3_CPU_H_ */
