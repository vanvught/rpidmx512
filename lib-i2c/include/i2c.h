/**
 * @file i2c.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(__linux__)
 #include "bcm2835.h"
#elif defined(H3)
 #include "h3_i2c.h"
#else
 #include "bcm2835_i2c.h"
#endif

typedef enum {
	I2C_NORMAL_SPEED = 100000,
	I2C_FULL_SPEED = 400000
} I2CBaudrate;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(H3)
 	#define FUNC_PREFIX(x) h3_##x

	inline static void i2c_set_address(uint8_t address) {
		h3_i2c_set_slave_address(address);
	}
#else
 	#define FUNC_PREFIX(x) bcm2835_##x

	inline static void i2c_set_address(uint8_t address) {
		bcm2835_i2c_setSlaveAddress(address);
	}
#endif

inline static bool i2c_begin(void) {
	FUNC_PREFIX(i2c_begin());
	return true;
}

inline static void i2c_set_baudrate(uint32_t baudrate) {
	FUNC_PREFIX(i2c_set_baudrate(baudrate));
}

inline static uint8_t i2c_read(char *buf, uint32_t len) {
	return FUNC_PREFIX(i2c_read(buf, len));
}

extern uint8_t i2c_read_uint8(void);
extern uint16_t i2c_read_uint16(void);
extern uint8_t i2c_read_reg_uint8(uint8_t);
extern uint16_t i2c_read_reg_uint16(uint8_t);
extern uint16_t i2c_read_reg_uint16_delayus(uint8_t, uint32_t);

#if defined(__linux__) || defined(H3)
	inline static void i2c_write(uint8_t data) {
		(void) FUNC_PREFIX(i2c_write((char *)&data, 1));
	}
#else
	extern void i2c_write(uint8_t);
#endif

inline static void i2c_write_nb(const char *data, uint32_t length) {
	(void) FUNC_PREFIX(i2c_write(data, length));
}

extern void i2c_write_reg_uint8(uint8_t, uint8_t);
extern void i2c_write_uint16(uint16_t);
extern void i2c_write_reg_uint16(uint8_t, uint16_t);
extern void i2c_write_reg_uint16_mask(uint8_t, uint16_t, uint16_t);

extern bool i2c_is_connected(uint8_t);

extern /*@observer@*/const char *i2c_lookup_device(uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
