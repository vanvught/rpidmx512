/**
 * @file hal_i2c.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hal_api.h"

#if defined(__linux__) || defined (__APPLE__)
# include "linux/hal_api.h"
# include "linux/hal_i2c.h"
#elif defined(H3)
# include "h3/hal_api.h"
# include "h3/hal_i2c.h"
#elif defined(GD32)
# include "gd32/hal_api.h"
# include "gd32/hal_i2c.h"
#else
# include "rpi/hal_api.h"
# include "rpi/hal_i2c.h"
#endif

#ifdef __cplusplus
#include <cstdint>

namespace hal {
namespace i2c {
static constexpr uint32_t NORMAL_SPEED = 100000;
static constexpr uint32_t FULL_SPEED = 400000;
}  // namespace i2c
}  // namespace hal

class HAL_I2C {
public:
	HAL_I2C(uint8_t nAddress, uint32_t nBaudrate = hal::i2c::FULL_SPEED) : m_nAddress(nAddress), m_nBaudrate(nBaudrate) {
	}

	uint8_t GetAddress() {
		return m_nAddress;
	}

	uint32_t GetBaudrate() {
		return m_nBaudrate;
	}

	bool IsConnected() {
		return IsConnected_(m_nAddress, m_nBaudrate);
	}

	static bool IsConnected(const uint8_t nAddress, uint32_t nBaudrate = hal::i2c::NORMAL_SPEED) {
		return IsConnected_(nAddress, nBaudrate);
	}

	void Write(uint8_t pData) {
		Setup();
		const char buffer[] = { static_cast<char>(pData) };
		FUNC_PREFIX(i2c_write(buffer, 1));
	}

	void Write(const char *pData, uint32_t nLength) {
		Setup();
		FUNC_PREFIX(i2c_write(pData, nLength));
	}

	void WriteRegister(uint8_t nRegister, uint8_t nValue) {
		const char buffer[] = {
			static_cast<char>(nRegister),
			static_cast<char>(nValue)
		};

		Setup();
		FUNC_PREFIX(i2c_write(buffer, 2));
	}

	void WriteRegister(uint8_t nRegister, uint16_t nValue) {
		const char buffer[] = {
			static_cast<char>(nRegister),
			static_cast<char>(nValue >> 8),
			static_cast<char>(nValue & 0xFF)
		};

		Setup();
		FUNC_PREFIX(i2c_write(buffer, 3));
	}

	uint8_t Read() {
		char buf[1] = {0};

		Setup();
		FUNC_PREFIX(i2c_read(buf, 1));

		return static_cast<uint8_t>(buf[0]);
	}

	uint8_t Read(char *pBuffer, uint32_t nLength) {
		Setup();
		return FUNC_PREFIX(i2c_read(pBuffer, nLength));
	}

	uint16_t Read16() {
		char buf[2] = {0};

		Setup();
		FUNC_PREFIX(i2c_read(buf, 2));

		return static_cast<uint16_t>(static_cast<uint16_t>(buf[0]) << 8 | static_cast<uint16_t>(buf[1]));
	}

	uint8_t ReadRegister(uint8_t nRegister) {
		const char buf[] = { static_cast<char>(nRegister) };

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		return Read();
	}

	uint16_t ReadRegister16(uint8_t nRegister) {
		const char buf[] = { static_cast<char>(nRegister) };

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		return Read16();
	}

	uint16_t ReadRegister16DelayUs(uint8_t nRegister, uint32_t nDelayUs) {
		char buf[2] = {0};

		buf[0] = static_cast<char>(nRegister);

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		udelay(nDelayUs);

		FUNC_PREFIX(i2c_read(buf, 2));

		return static_cast<uint16_t>(static_cast<uint16_t>(buf[0]) << 8 | static_cast<uint16_t>(buf[1]));
	}

	bool AckRead() {
		char buf;
		return FUNC_PREFIX(i2c_read(&buf, 1)) == 0;
	}

private:
	void Setup() {
		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(m_nBaudrate));
	}

	static bool IsConnected_(const uint8_t nAddress, uint32_t nBaudrate) {
		char buf;

		FUNC_PREFIX(i2c_set_address(nAddress));
		FUNC_PREFIX(i2c_set_baudrate(nBaudrate));

		if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
			return FUNC_PREFIX(i2c_read(&buf, 1)) == 0;
		}

		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		return FUNC_PREFIX(i2c_write(nullptr, 0)) == 0;
	}

	uint8_t m_nAddress;
	uint32_t m_nBaudrate;
};

#endif

#endif /* HAL_I2C_H_ */
