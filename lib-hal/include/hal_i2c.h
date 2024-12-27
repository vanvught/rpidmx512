/**
 * @file hal_i2c.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (CONFIG_I2C_OPTIMIZE_O2) || defined (CONFIG_I2C_OPTIMIZE_O3)
# pragma GCC push_options
# if defined (CONFIG_I2C_OPTIMIZE_O2)
#  pragma GCC optimize ("O2")
# else
#  pragma GCC optimize ("O3")
# endif
#endif

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

/**
 * @class HAL_I2C
 * @brief A class for managing I2C communication in an embedded system.
 *
 * The `HAL_I2C` class provides a convenient API for interacting with I2C devices.
 * It supports various operations such as reading, writing, and checking the
 * connectivity of an I2C device. Default baud rates are provided but can be
 * customized as needed.
 *
 * @note Designed for Cortex-M3/M4 embedded systems without a standard library.
 */
class HAL_I2C {
public:
	static constexpr uint32_t NORMAL_SPEED = 100000;
	static constexpr uint32_t FULL_SPEED = 400000;

	/**
	 * @brief Constructs a new HAL_I2C object.
	 *
	 * @param nAddress The I2C address of the device.
	 * @param nBaudrate The I2C baud rate (default: `HAL_I2C::FULL_SPEED`).
	 *
	 * @note The I2C address is 7-bit. Use `HAL_I2C::NORMAL_SPEED` for slower communication.
	 */
	HAL_I2C(uint8_t nAddress, uint32_t nBaudrate = FULL_SPEED) : m_nAddress(nAddress), m_nBaudrate(nBaudrate) {}

	/**
	 * @brief Gets the I2C device address.
	 *
	 * @return The 7-bit I2C address.
	 */
	uint8_t GetAddress() const {
		return m_nAddress;
	}

	/**
	 * @brief Gets the current I2C baud rate.
	 *
	 * @return The baud rate in Hz.
	 */
	uint32_t GetBaudrate() const {
		return m_nBaudrate;
	}

	/**
	 * @brief Checks if the I2C device is connected.
	 *
	 * This function verifies the connection to the specified I2C device address
	 * using the configured baud rate.
	 *
	 * @return `true` if the device acknowledges communication, `false` otherwise.
	 */
	bool IsConnected() {
		return FUNC_PREFIX(i2c_is_connected(m_nAddress, m_nBaudrate));
	}

	/**
	 * @brief Static method to check if an I2C device is connected.
	 *
	 * @param nAddress The I2C address of the device.
	 * @param nBaudrate The I2C baud rate (default: `HAL_I2C::NORMAL_SPEED`).
	 * @return `true` if the device acknowledges communication, `false` otherwise.
	 */
	static bool IsConnected(const uint8_t nAddress, uint32_t nBaudrate = NORMAL_SPEED) {
		return FUNC_PREFIX(i2c_is_connected(nAddress, nBaudrate));
	}

	/**
	 * @brief Writes a single byte to the I2C device.
	 *
	 * @param pData The byte to write.
	 */
	void Write(const uint8_t pData) {
		Setup();
		const char buffer[] = { static_cast<char>(pData) };
		FUNC_PREFIX(i2c_write(buffer, 1));
	}

	/**
	 * @brief Writes multiple bytes to the I2C device.
	 *
	 * @param pData Pointer to the data buffer.
	 * @param nLength Number of bytes to write.
	 */
	void Write(const char *pData, const uint32_t nLength) {
		Setup();
		FUNC_PREFIX(i2c_write(pData, nLength));
	}

	/**
	 * @brief Writes a single byte to a specific register of the I2C device.
	 *
	 * @param nRegister The register address.
	 * @param nValue The value to write.
	 */
	void WriteRegister(const uint8_t nRegister, const uint8_t nValue) {
		const char buffer[] =
			{ static_cast<char>(nRegister),
			  static_cast<char>(nValue) };

		Setup();
		FUNC_PREFIX(i2c_write(buffer, 2));
	}

	/**
	 * @brief Writes a 16-bit value to a specific register of the I2C device.
	 *
	 * @param nRegister The register address.
	 * @param nValue The 16-bit value to write.
	 */
	void WriteRegister(const uint8_t nRegister, const uint16_t nValue) {
		const char buffer[] =
			{ static_cast<char>(nRegister),
			  static_cast<char>(nValue >> 8),
			  static_cast<char>(nValue & 0xFF) };

		Setup();
		FUNC_PREFIX(i2c_write(buffer, 3));
	}

	/**
	 * @brief Reads a single byte from the I2C device.
	 *
	 * @return The byte read from the device.
	 */
	uint8_t Read() {
		char buf[1] = { 0 };

		Setup();
		FUNC_PREFIX(i2c_read(buf, 1));

		return static_cast<uint8_t>(buf[0]);
	}

	/**
	 * @brief Reads multiple bytes from the I2C device.
	 *
	 * @param pBuffer Pointer to the buffer where data will be stored.
	 * @param nLength Number of bytes to read.
	 * @return The number of bytes successfully read.
	 */
	uint8_t Read(char *pBuffer, const uint32_t nLength) {
		Setup();
		return FUNC_PREFIX(i2c_read(pBuffer, nLength));
	}

	/**
	 * @brief Reads a 16-bit value from the I2C device.
	 *
	 * @return The 16-bit value read from the device.
	 */
	uint16_t Read16() {
		char buf[2] = { 0 };

		Setup();
		FUNC_PREFIX(i2c_read(buf, 2));

		return static_cast<uint16_t>(static_cast<uint16_t>(buf[0]) << 8 | static_cast<uint16_t>(buf[1]));
	}

	/**
	 * @brief Reads a single byte from a specific register of the I2C device.
	 *
	 * @param nRegister The register address.
	 * @return The byte read from the register.
	 */
	uint8_t ReadRegister(const uint8_t nRegister) {
		const char buf[] = { static_cast<char>(nRegister) };

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		return Read();
	}

	/**
	 * @brief Reads a 16-bit value from a specific register of the I2C device.
	 *
	 * @param nRegister The register address.
	 * @return The 16-bit value read from the register.
	 */
	uint16_t ReadRegister16(const uint8_t nRegister) {
		const char buf[] = { static_cast<char>(nRegister) };

		Setup();
		FUNC_PREFIX(i2c_write(&buf[0], 1));

		return Read16();
	}

	uint16_t ReadRegister16DelayUs(const uint8_t nRegister, const uint32_t nDelayUs) {
		char buf[2] = { 0 };

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
	/**
	 * @brief Configures the I2C address and baud rate.
	 *
	 * This function is called internally before any I2C operation to ensure
	 * the correct address and baud rate are set.
	 */
	void Setup() {
		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(m_nBaudrate));
	}

	uint8_t m_nAddress;
	uint32_t m_nBaudrate;
};
#endif

#if defined (CONFIG_I2C_OPTIMIZE_O2) || defined (CONFIG_I2C_OPTIMIZE_O3)
# pragma GCC pop_options
#endif

#endif /* HAL_I2C_H_ */
