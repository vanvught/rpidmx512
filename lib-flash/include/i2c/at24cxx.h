/**
 * @file  at24cxx.h
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

namespace at24cxx {
static constexpr uint8_t I2C_ADDRESS = 0x50;
}  // namespace at24cxx

class AT24Cxx  {
public:
	AT24Cxx(uint8_t nAddress, uint8_t nPageSize) : m_I2C(nAddress), m_nPageSize(nPageSize) {
		m_IsConnected =  m_I2C.IsConnected();
	}

	bool IsConnected() const {
		return m_IsConnected;
	}

	uint8_t GetAddress() {
		return m_I2C.GetAddress();
	}

	uint8_t GetPageSize() const {
		return m_nPageSize;
	}

	void Write(uint32_t nAddress, uint8_t nData) {
		if (!m_IsConnected) {
			return;
		}

		while (!m_I2C.AckRead());

		const char buffer[] = {
			static_cast<char>(nAddress >> 8),
			static_cast<char>(nAddress & 0xFF),
			static_cast<char>(nData)
		};

		m_I2C.Write(buffer, 3);
	}

	void Write(uint32_t nAddress, const uint8_t *pData, uint32_t nLength) {
		if (!m_IsConnected) {
			return;
		}

		char buffer[32];
		uint32_t nIndex = 0;

		while (nLength > 0) {
			while (!m_I2C.AckRead());

			const auto nOffsetPage = nAddress % m_nPageSize;
			const auto nCount = std::min(std::min(nLength, static_cast<uint32_t>(30)), m_nPageSize - nOffsetPage);

			buffer[0] = static_cast<char>(nAddress >> 8);
			buffer[1] = static_cast<char>(nAddress & 0xFF);

			memcpy(&buffer[2], &pData[nIndex], nCount);
			m_I2C.Write(buffer, 2 + nCount);

			nLength = nLength - nCount;
			nAddress = nAddress + nCount;
			nIndex = nIndex + nCount;
		}
	}

	uint8_t Read(uint32_t nAddress) {
		if (!m_IsConnected) {
			return 1;
		}

		while (!m_I2C.AckRead());

		const char buffer[] = { static_cast<char>(nAddress >> 8), static_cast<char>(nAddress & 0xFF) };
		m_I2C.Write(buffer, 2);

		return m_I2C.Read();
	}

	uint8_t Read(uint32_t nAddress, uint8_t *pData, uint32_t nLength) {
		if (!m_IsConnected) {
			return 1;
		}

		while (!m_I2C.AckRead());

		const char buffer[] = {
			static_cast<char>(nAddress >> 8),
			static_cast<char>(nAddress & 0xFF)
		};

		m_I2C.Write(buffer, 2);

		return m_I2C.Read(reinterpret_cast<char *>(pData), nLength);
	}

private:
	HAL_I2C m_I2C;
	bool m_IsConnected { false };
	uint8_t m_nPageSize;
};

class AT24C32: public AT24Cxx {
public:
	AT24C32(const uint8_t nIndex) : AT24Cxx(at24cxx::I2C_ADDRESS + (nIndex & 0x7), 32) {
	}
};

#endif /* I2C_AT24CXX_H_ */
