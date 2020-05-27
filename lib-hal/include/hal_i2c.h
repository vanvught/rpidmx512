/**
 * @file hal_i2c.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef HAL_I2C_H_
#define HAL_I2C_H_

enum {
	I2C_NORMAL_SPEED = 100000,
	I2C_FULL_SPEED = 400000
};

#if defined(__linux__)
# include "linux/hal_i2c.h"
# include "linux/hal_api.h"
#elif defined(H3)
# include "h3/hal_i2c.h"
# include "h3/hal_api.h"
#else
# include "rpi/hal_i2c.h"
# include "rpi/hal_api.h"
#endif

#include <stdint.h>

class HAL_I2C {
public:
	HAL_I2C(uint8_t nAddress, uint32_t nBaudrate = I2C_FULL_SPEED) : m_nAddress(nAddress), m_nBaudrate(nBaudrate) {
	}

	bool IsConnected(void) {
		Setup();

		char buf;

		if ((m_nAddress >= 0x30 && m_nAddress <= 0x37) || (m_nAddress >= 0x50 && m_nAddress <= 0x5F)) {
			return FUNC_PREFIX(i2c_read(&buf, 1)) == 0;
		}

		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		return FUNC_PREFIX(i2c_write(0, 0)) == 0;
	}

	void WriteRegister(uint8_t nRegister, uint8_t nValue) {
		char buffer[2];

		buffer[0] = nRegister;
		buffer[1] = nValue;

		Setup();
		FUNC_PREFIX(i2c_write(buffer, 2));
	}

	uint8_t Read(void) {
		char buf[1];

		Setup();
		FUNC_PREFIX(i2c_read(buf, 1));

		return buf[0];
	}

	uint8_t ReadRegister(uint8_t nRegister) {
		char buf[2];

		buf[0] = nRegister;
		buf[1] = 0;

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		return Read();
	}

private:
	void Setup(void) {
		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(m_nBaudrate));
	}

	uint8_t m_nAddress;
	uint32_t m_nBaudrate;
};

#endif /* HAL_I2C_H_ */
