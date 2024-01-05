/**
 * @file sc16is740.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SC16IS740_H_
#define SC16IS740_H_

#include <cstdint>

#include "hal_i2c.h"
#include "hardware.h"

#include "sc16is7x0.h"

namespace sc16is740 {
static constexpr uint8_t I2C_ADDRESS = 0x4D;
static constexpr uint32_t CRISTAL_HZ = 14745600UL;
}

class SC16IS740: HAL_I2C {
public:
	enum class SerialParity {
		NONE, ODD, EVEN, FORCED0, FORCED1
	};

	enum class TriggerLevel {
		LEVEL_TX, LEVEL_RX
	};

	SC16IS740(uint8_t nAddress = sc16is740::I2C_ADDRESS, uint32_t nOnBoardCrystal = sc16is740::CRISTAL_HZ);
	~SC16IS740() = default;

	void SetOnBoardCrystal(uint32_t nOnBoardCrystalHz) {
		m_nOnBoardCrystal = nOnBoardCrystalHz;
	}

	uint32_t GetOnBoardCrystal() const {
		return m_nOnBoardCrystal;
	}

	void SetFormat(uint32_t nBits, SerialParity tParity, uint32_t nStopBits);
	void SetBaud(uint32_t nBaud);

	bool IsInterrupt() {
		const uint32_t nRegisterIIR = ReadRegister(SC16IS7X0_IIR);

		return ((nRegisterIIR & 0x1) != 0x1);
	}

	// Read

	int GetChar() {
		if (!m_IsConnected) {
			return -1;
		}

		if (!IsReadable()) {
			return -1;
		}

		return ReadRegister(SC16IS7X0_RHR);
	}

	int GetChar(uint32_t nTimeOut) {
		if (!m_IsConnected) {
			return -1;
		}

		if (!IsReadable(nTimeOut)) {
			return -1;
		}

		return ReadRegister(SC16IS7X0_RHR);
	}

	// Write
	int PutChar(int nValue) {
		if (!m_IsConnected) {
			return -1;
		}

		while (!IsWritable()) {
		}

		WriteRegister(SC16IS7X0_THR, static_cast<uint8_t>(nValue));

		return nValue;
	}

	// Multiple read/write

	void WriteBytes(const uint8_t *pBytes, uint32_t nSize);
	void ReadBytes(uint8_t *pBytes, uint32_t& nSize, uint32_t nTimeOut);
	void FlushRead(uint32_t nTimeOut);

private:
	bool IsWritable() {
		return (ReadRegister(SC16IS7X0_TXLVL) != 0);
	}

	bool IsReadable() {
		return (ReadRegister(SC16IS7X0_RXLVL) != 0);
	}

	bool IsReadable(uint32_t nTimeOut) {
		const auto nMillis = Hardware::Get()->Millis();
		do {
			if (IsReadable()) {
				return true;
			}
		} while ((Hardware::Get()->Millis() - nTimeOut) < nMillis);

		return false;
	}

private:
	uint32_t m_nOnBoardCrystal;
	bool m_IsConnected { false };
};

#endif /* SC16IS740_H_ */
