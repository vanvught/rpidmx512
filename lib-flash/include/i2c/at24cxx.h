/**
 * @file  at24cxx.h
 * @brief I2C interface for AT24Cxx EEPROM devices.
 *
 * This header defines a templated class for interfacing with
 * AT24Cxx EEPROM devices over I2C, supporting multiple EEPROM types.
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef I2C_AT24CXX_H_
#define I2C_AT24CXX_H_

#include <cstdint>
#include <cstring>
#include <algorithm>

#include "hal_i2c.h"

/**
 * @namespace at24cxx
 * @brief Contains constants and types for AT24Cxx EEPROM devices.
 */
namespace at24cxx {
static constexpr uint8_t I2C_ADDRESS = 0x50;

/**
 * @struct ATTypes
 * @brief Defines the sizes of different AT24Cxx EEPROM devices in bytes.
 */
struct ATTypes {
	static constexpr uint32_t AT24LC512 = 65536;
	static constexpr uint32_t AT24LC256 = 32768;
	static constexpr uint32_t AT24LC128 = 16384;
	static constexpr uint32_t AT24LC64  = 8192;
	static constexpr uint32_t AT24LC32  = 4096;
	static constexpr uint32_t AT24LC16  = 2048;
	static constexpr uint32_t AT24LC08  = 1024;
	static constexpr uint32_t AT24LC04  = 512;
	static constexpr uint32_t AT24LC02  = 256;
	static constexpr uint32_t AT24LC01  = 128;
};
}  // namespace at24cxx

/**
 * @class AT24Cxx
 * @brief Template class for interfacing with AT24Cxx EEPROM devices.
 *
 * This class provides methods to read and write to AT24Cxx devices
 * using I2C communication. The `type` parameter defines the EEPROM
 * type and size.
 *
 * @tparam type Size of the EEPROM in bytes.
 */
template<uint32_t type>
class AT24Cxx  {
	static constexpr bool IsValidType() {
		return type == at24cxx::ATTypes::AT24LC512 ||
		       type == at24cxx::ATTypes::AT24LC256 ||
		       type == at24cxx::ATTypes::AT24LC128 ||
		       type == at24cxx::ATTypes::AT24LC64 ||
		       type == at24cxx::ATTypes::AT24LC32 ||
		       type == at24cxx::ATTypes::AT24LC16 ||
		       type == at24cxx::ATTypes::AT24LC08 ||
		       type == at24cxx::ATTypes::AT24LC04 ||
		       type == at24cxx::ATTypes::AT24LC02 ||
		       type == at24cxx::ATTypes::AT24LC01;
	}
public:
    /**
     * @brief Constructor for AT24Cxx.
     *
     * @param nSlaveAddress I2C slave address of the EEPROM device.
     */
	AT24Cxx(uint8_t nSlaveAddress) : m_nSlaveAddress(nSlaveAddress) {
		static_assert(IsValidType(), "Invalid type specified for AT24Cxx.");
		m_IsConnected = FUNC_PREFIX(I2cIsConnected(m_nSlaveAddress, 400000));;
	}

	/** @brief Checks if the EEPROM device is connected. */
	bool IsConnected() const {
		return m_IsConnected;
	}

	/** @brief Returns the I2C address of the EEPROM device. */
	uint8_t GetAddress() const {
		return m_nSlaveAddress;
	}

	/** @brief Returns the size of the EEPROM device. */
	constexpr uint32_t GetSize() {
		return type;
	}

    /**
     * @brief Returns the page size of the EEPROM device.
     *
     * Page size varies depending on the EEPROM type.
     */
	constexpr uint32_t GetPageSize() {
	    if constexpr (type <= at24cxx::ATTypes::AT24LC02) return 8;
	    if constexpr (type <= at24cxx::ATTypes::AT24LC16) return 16;
	    if constexpr (type <= at24cxx::ATTypes::AT24LC64) return 32;
	    if constexpr (type <= at24cxx::ATTypes::AT24LC256) return 64;
	    return 128;
	}

    /**
     * @brief Writes a single byte to a specific memory address.
     *
     * @param nMemoryAddress Address in EEPROM memory.
     * @param nData Byte to be written.
     */
	void Write(const uint32_t nMemoryAddress, const uint8_t nData) {
		if (!m_IsConnected) {
			return;
		}

		FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress));

		while (!AckRead())
			;

		if constexpr (isAddressSizeTwoWords) {
			const char buffer[] =
					{ static_cast<char>(nMemoryAddress >> 8),
					  static_cast<char>(nMemoryAddress & 0xFF),
					  static_cast<char>(nData)
					};
			FUNC_PREFIX(I2cWrite(buffer, (sizeof(buffer) / sizeof(buffer[0]))));
		} else {
			const char buffer[] =
					{ static_cast<char>(nMemoryAddress & 0xFF),
					  static_cast<char>(nData)
					};
			FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress | ((nMemoryAddress >> 8) & 0x7)));
			FUNC_PREFIX(I2cWrite(buffer, (sizeof(buffer) / sizeof(buffer[0]))));
		}
	}

    /**
     * @brief Writes multiple bytes starting from a specific memory address.
     *
     * @param nMemoryAddress Starting address in EEPROM memory.
     * @param pData Pointer to the data buffer.
     * @param nLength Number of bytes to write.
     */
	void Write(uint32_t nMemoryAddress, const uint8_t *pData, uint32_t nLength) {
		if (!m_IsConnected) {
			return;
		}

		char buffer[128];
		uint32_t nIndex = 0;

		FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress));

		while (nLength > 0) {
			while (!AckRead());

			const auto nOffsetPage = nMemoryAddress % GetPageSize();
			uint32_t nCount;

			if constexpr (isAddressSizeTwoWords) {
				nCount = std::min(std::min(nLength, GetPageSize() - 2), GetPageSize() - nOffsetPage);
				buffer[0] = static_cast<char>(nMemoryAddress >> 8);
				buffer[1] = static_cast<char>(nMemoryAddress & 0xFF);

				memcpy(&buffer[2], &pData[nIndex], nCount);
				FUNC_PREFIX(I2cWrite(buffer, 2 + nCount));
			} else {
				nCount = std::min(std::min(nLength, GetPageSize() - 1), GetPageSize() - nOffsetPage);
				buffer[0] = static_cast<char>(nMemoryAddress & 0xFF);
				memcpy(&buffer[1], &pData[nIndex], nCount);

				FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress | ((nMemoryAddress >> 8) & 0x7)));
				FUNC_PREFIX(I2cWrite(buffer, 1 + nCount));
			}

			nLength -= nCount;
			nMemoryAddress += nCount;
			nIndex += nCount;
		}
	}

    /**
     * @brief Reads a single byte from a specific memory address.
     *
     * @param nMemoryAddress Address in EEPROM memory.
     * @return The byte read from memory.
     */
	uint8_t Read(const uint32_t nMemoryAddress) {
		if (!m_IsConnected) {
			return 0;
		}

		FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress));

		while (!AckRead());

		if constexpr (isAddressSizeTwoWords) {
			char buffer[] =
				{ static_cast<char>(nMemoryAddress >> 8),
				  static_cast<char>(nMemoryAddress & 0xFF)
				};
			FUNC_PREFIX(I2cWrite(buffer, sizeof(buffer) / sizeof(buffer[0])));
		} else {
			const char buffer[] = { static_cast<char>(nMemoryAddress & 0xFF) };
			FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress | ((nMemoryAddress >> 8) & 0x7)));
			FUNC_PREFIX(I2cWrite(buffer, sizeof(buffer) / sizeof(buffer[0])));
 		}

		char c;
		FUNC_PREFIX(I2cRead(&c, 1));
		return static_cast<uint8_t>(c);
	}

    /**
     * @brief Reads multiple bytes starting from a specific memory address.
     *
     * @param nMemoryAddress Starting address in EEPROM memory.
     * @param pData Pointer to the buffer to store data.
     * @param nLength Number of bytes to read.
     * @return Status of the operation (0 for success, 1 for failure).
     */
	uint8_t Read(const uint32_t nMemoryAddress, uint8_t *pData, const uint32_t nLength) {
		if (!m_IsConnected) {
			return 1;
		}

		FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress));

		while (!AckRead());

		if constexpr (isAddressSizeTwoWords) {
			const char buffer[] =
				{ static_cast<char>(nMemoryAddress >> 8),
				  static_cast<char>(nMemoryAddress & 0xFF)
				};

			FUNC_PREFIX(I2cWrite(buffer, sizeof(buffer) / sizeof(buffer[0])));
		} else {
			const char buffer[] = { static_cast<char>(nMemoryAddress & 0xFF) };
			FUNC_PREFIX(I2cSetAddress(m_nSlaveAddress | ((nMemoryAddress >> 8) & 0x7)));
			FUNC_PREFIX(I2cWrite(buffer, sizeof(buffer) / sizeof(buffer[0])));
		}

		return FUNC_PREFIX(I2cRead(reinterpret_cast<char *>(pData), nLength));
	}

private:
	/** @brief Performs an ACK read operation. */
	bool AckRead() {
		char c;
		return FUNC_PREFIX(I2cRead(&c, 1)) == 0;
	}

    /** @brief Determines if the memory address size is 2 bytes. */
	static constexpr bool isAddressSizeTwoWords = type > at24cxx::ATTypes::AT24LC16;

private:
	uint8_t m_nSlaveAddress;
	bool m_IsConnected { false };
};

/**
 * @class AT24C04
 * @brief Specialized class for the AT24C04 EEPROM.
 */
class AT24C04 : public AT24Cxx<at24cxx::ATTypes::AT24LC04> {
public:
	AT24C04()
        : AT24Cxx(at24cxx::I2C_ADDRESS) {}
};

/**
 * @class AT24C32
 * @brief Specialized class for the AT24C32 EEPROM.
 */
class AT24C32 : public AT24Cxx<at24cxx::ATTypes::AT24LC32> {
public:
    /**
     * @brief Constructor for AT24C32.
     *
     * @param nIndex Index to select the appropriate I2C address.
     */
    AT24C32(uint8_t nIndex)
        : AT24Cxx(at24cxx::I2C_ADDRESS + (nIndex & 0x7)) {}
};

#endif /* I2C_AT24CXX_H_ */
