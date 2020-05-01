/**
 * @file si5351a.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SI5351A_H_
#define SI5351A_H_

#include <stdbool.h>

#include "device_info.h"

#define SI5351A_I2C_DEFAULT_SLAVE_ADDRESS	0x60

#define SI5351A_REVB_REG_CONFIG_NUM_REGS	57

typedef struct {
	unsigned int address; /* 16-bit register address */
	unsigned char value; /* 8-bit register data */

} si5351a_revb_register_t;

#ifdef __cplusplus
extern "C" {
#endif

extern bool si5351a_start(device_info_t *);

extern bool si5351a_clock_builder(const device_info_t *);
#if defined (__linux__)
extern bool si5351a_csv(const device_info_t *, const char *);
#endif

extern void si5351a_dump(device_info_t *);

#ifdef __cplusplus
}
#endif

#endif /* SI5351A_H_ */
