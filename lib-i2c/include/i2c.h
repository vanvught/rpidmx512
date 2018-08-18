/**
 * @file i2c.h
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


#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	I2C_NORMAL_SPEED = 100000,
	I2C_FULL_SPEED = 400000
}I2CBaudrate;

typedef enum {
	I2C_CLOCK_DIVIDER_100kHz	= 2500,		///< 2500 = 10us = 100 kHz
	I2C_CLOCK_DIVIDER_400kHz	= 626		///< 622 = 2.504us = 399.3610 kHz
} I2CClockDivider;

#ifdef __cplusplus
extern "C" {
#endif

extern bool i2c_begin(void);
extern void i2c_set_address(uint8_t);
extern void i2c_set_clockdivider(uint16_t);		// // Obsolete - Backwards compatibility with Raspberry Pi
extern void i2c_set_baudrate(uint32_t);

extern bool i2c_is_connected(uint8_t);

extern uint8_t i2c_read(char *, uint32_t);
extern uint8_t i2c_read_uint8(void);
extern uint16_t i2c_read_uint16(void);
extern uint16_t i2c_read_reg_uint16(uint8_t);
extern uint16_t i2c_read_reg_uint16_delayus(uint8_t, uint32_t);

extern void i2c_write(uint8_t);
extern void i2c_write_nb(const char *, uint32_t);
extern void i2c_write_reg_uint8(uint8_t, uint8_t);
extern void i2c_write_uint16(uint16_t);
extern void i2c_write_reg_uint16(uint8_t, uint16_t);
extern void i2c_write_reg_uint16_mask(uint8_t, uint16_t, uint16_t);

extern /*@observer@*/const char *i2c_lookup_device(uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
