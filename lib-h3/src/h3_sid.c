/**
 * @file h3_sid.c
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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3.h"

#define	SID_PRCTL_OFFSET_MASK	0xFF
#define SID_PRCTL_OFFSET(n)		(((n) & SID_PRCTL_OFFSET_MASK) << 16)
#define	SID_PRCTL_LOCK			(0xAC << 8)
#define	SID_PRCTL_READ			(0x01 << 1)
#define	SID_PRCTL_WRITE			(0x01 << 0)

#define	SID_THERMAL_CALIB_OFFSET	0x34

#define	ROOT_KEY_SIZE	4

static inline void be32enc(void *pp, uint32_t u) {
	uint8_t *p = (uint8_t *) pp;

	p[0] = (u >> 24) & 0xff;
	p[1] = (u >> 16) & 0xff;
	p[2] = (u >> 8) & 0xff;
	p[3] = u & 0xff;
}

static void prctl_read(uint32_t offset, uint32_t *val) {
	H3_SID->PRCTL = SID_PRCTL_OFFSET(offset) | SID_PRCTL_LOCK | SID_PRCTL_READ;
	/* Read bit will be cleared once read has concluded */
	while (H3_SID->PRCTL & SID_PRCTL_READ) {
		continue;
	}

	*val = H3_SID->RDKEY;
}

void h3_sid_get_rootkey(uint8_t *out) {
	int i;
	uint32_t tmp;

	for (i = 0; i < ROOT_KEY_SIZE; i++) {
		prctl_read((i * 4), &tmp);
		be32enc(&out[i * 4], tmp);
	}
}

void h3_sid_read_tscalib(uint32_t *calib) {
	prctl_read(SID_THERMAL_CALIB_OFFSET, calib);
}
