/**
 * @file h3_i2c.h
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

#ifndef H3_I2C_H_
#define H3_I2C_H_

#include <stdint.h>

typedef enum H3_I2C_BAUDRATE {
	H3_I2C_NORMAL_SPEED = 100000,
	H3_I2C_FULL_SPEED = 400000
} h3_i2c_baudrate_t;

typedef enum H2_I2C_RC {
	H3_I2C_OK = 0,
	H3_I2C_NOK = 1,
	H3_I2C_NACK = 2,
	H3_I2C_NOK_LA = 3,
	H3_I2C_NOK_TOUT = 4
} h3_i2c_rc_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_i2c_begin(void);
extern void h3_i2c_end(void);
extern uint8_t h3_i2c_write(/*@null@*/const char *, uint32_t);
extern uint8_t h3_i2c_read(/*@out@*/char *, uint32_t);
extern void h3_i2c_set_baudrate(uint32_t);
extern void h3_i2c_set_slave_address(uint8_t);

// Obsolete - Backwards compatibility with Raspberry Pi
#define h3_i2c_setSlaveAddress	h3_i2c_set_slave_address	//TODO remove h3_i2c_setSlaveAddress
extern void h3_i2c_setClockDivider(uint16_t);				//TODO remove h3_i2c_setClockDivider

#ifdef __cplusplus
}
#endif

#endif /* H3_I2C_H_ */
