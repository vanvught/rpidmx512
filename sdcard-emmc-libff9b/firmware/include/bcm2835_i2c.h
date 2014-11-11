/**
 * @file bcm2835_i2c.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BCM2835_I2C_H_
#define BCM2835_I2C_H_

#include <stdint.h>

typedef enum {
	BCM2835_I2C_CLOCK_DIVIDER_2500	= 2500,		///< 2500 = 10us = 100 kHz
	BCM2835_I2C_CLOCK_DIVIDER_626	= 626,		///< 622 = 2.504us = 399.3610 kHz
	BCM2835_I2C_CLOCK_DIVIDER_150	= 150,		///< 150 = 60ns = 1.666 MHz (default at reset)
	BCM2835_I2C_CLOCK_DIVIDER_148	= 148,		///< 148 = 59ns = 1.689 MHz
} bcm2835I2CClockDivider;

typedef enum {
	BCM2835_I2C_REASON_OK			= 0x00,		///< Success
	BCM2835_I2C_REASON_ERROR_NACK 	= 0x01,		///< Received a NACK
	BCM2835_I2C_REASON_ERROR_CLKT 	= 0x02,		///< Received Clock Stretch Timeout
	BCM2835_I2C_REASON_ERROR_DATA 	= 0x04		///< Not all data is sent / received
} bcm2835I2CReasonCodes;

extern void bcm2835_i2c_begin(void);
extern void bcm2835_i2c_end(void);
extern void bcm2835_i2c_setSlaveAddress(const uint8_t);
extern void bcm2835_i2c_setClockDivider(const uint16_t);
extern uint8_t bcm2835_i2c_write(const char *, const uint32_t);
extern uint8_t bcm2835_i2c_read(char*, const uint32_t);

#endif /* BCM2835_I2C_H_ */
