#if defined(HAVE_I2C)
/**
 * @file bw_i2c.c
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

#include <stdint.h>
#include <stddef.h>

extern void udelay(uint32_t);

#include "i2c.h"
#include "bw.h"
#include "device_info.h"

#define I2C_DELAY_WRITE_READ_US		100

void bw_i2c_read_id(const device_info_t *device_info, char *id) {
	i2c_set_address(device_info->slave_address >> 1);
	i2c_set_baudrate(I2C_NORMAL_SPEED);

	i2c_write(BW_PORT_READ_ID_STRING);
	udelay(I2C_DELAY_WRITE_READ_US);
	(void) i2c_read(id, BW_ID_STRING_LENGTH);
}
#endif
