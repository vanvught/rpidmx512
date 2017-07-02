/**
 * @file bcm2835_st.h
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

#ifndef BCM2835_ST_H_
#define BCM2835_ST_H_

#include "bcm2835.h"

#define BCM2835_ST_CS_M0		((uint32_t)(1 << 0))	///< System Timer Match 0. DO NOT USE; is used by GPU.
#define BCM2835_ST_CS_M1		((uint32_t)(1 << 1))	///< System Timer Match 1
#define BCM2835_ST_CS_M2		((uint32_t)(1 << 2))	///< System Timer Match 2. DO NOT USE; is used by GPU.
#define BCM2835_ST_CS_M3		((uint32_t)(1 << 3))	///< System Timer Match 3

#ifdef __cplusplus
extern "C" {
#endif

extern const uint64_t bcm2835_st_read(void);

#ifdef __cplusplus
}
#endif

#endif /* BCM2835_ST_H_ */
