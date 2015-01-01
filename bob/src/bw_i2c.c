/**
 * @file bw_i2c.c
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

#ifdef __AVR_ARCH__
#include <avr_i2c.h>
#else
#include <bcm2835.h>
#ifdef BARE_METAL
#include <bcm2835_i2c.h>
#endif
#endif
#include <device_info.h>
#include <bw.h>

extern int printf(const char *format, ...);

/**
 * @ingroup I2C
 *
 * @param device_info
 */
void bw_i2c_read_id(const device_info_t *device_info) {
	char cmd[] = { BW_PORT_READ_ID_STRING };
	char buf[BW_ID_STRING_LENGTH];
	FUNC_PREFIX(i2c_setSlaveAddress(device_info->slave_address >> 1));
#ifdef __AVR_ARCH__
#else
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
#endif
	FUNC_PREFIX(i2c_write(cmd, sizeof(cmd) / sizeof(char)));
	FUNC_PREFIX(i2c_read(buf, BW_ID_STRING_LENGTH));
	printf("[%s]\n", buf);
}
