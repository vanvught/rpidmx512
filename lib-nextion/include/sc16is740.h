/**
 * @file sc16is740.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>

#include "hardware.h"
#include "sc16is7x0.h"

enum {
	SC16IS740_I2C_ADDRESS = 0x4D
};

enum {
	SC16IS740_CRISTAL_HZ = 14745600UL
};

enum TSC16IS740SerialParity {
	SC16IS740_SERIAL_PARITY_NONE,
	SC16IS740_SERIAL_PARITY_ODD,
	SC16IS740_SERIAL_PARITY_EVEN,
	SC16IS740_SERIAL_PARITY_FORCED0,
	SC16IS740_SERIAL_PARITY_FORCED1
};

enum TSC16IS740TriggerLevel {
	SC16IS740_TRIGGER_LEVEL_TX,
	SC16IS740_TRIGGER_LEVEL_RX
};

class SC16IS740  {
public:
	SC16IS740(void);
	SC16IS740(uint8_t nAddress, uint32_t nOnBoardCrystal = SC16IS740_CRISTAL_HZ);
	~SC16IS740(void);

	void SetAddress(uint8_t nAddress) {
		m_nAddress = nAddress;
	}
	uint8_t GetAddress(void) {
		return m_nAddress;
	}

	void SetOnBoardCrystal(uint32_t nHz) {
		m_nOnBoardCrystal = nHz;
	}
	uint32_t GetOnBoardCrystal(void) {
		return m_nOnBoardCrystal;
	}

	bool Init(void);

	void SetFormat(uint32_t nBits, TSC16IS740SerialParity tParity, uint32_t nStopBits);
	void SetBaud(uint32_t nBaud);

	bool IsInterrupt(void) {
		const uint32_t nRegisterIIR = RegRead(SC16IS7X0_IIR);

		return ((nRegisterIIR & 0x1) != 0x1);
	}

	void Print(void);

	// Read

	int GetChar(void) {
		if (!IsReadable()) {
			return -1;
		}

		return RegRead(SC16IS7X0_RHR);
	}

	int GetChar(uint32_t nTimeOut) {
		if (!IsReadable(nTimeOut)) {
			return -1;
		}

		return RegRead(SC16IS7X0_RHR);
	}

	// Write
	int PutChar(int nValue) {
		while (!IsWritable()) {
		}

		RegWrite(SC16IS7X0_THR, nValue);

		return nValue;
	}

	// Multiple read/write

	void WriteBytes(const uint8_t *pBytes, uint32_t nSize);
	void ReadBytes(uint8_t *pBytes, uint32_t &nSize, uint32_t nTimeOut);
	void FlushRead(uint32_t nTimeOut);

private:
	bool IsWritable(void) {
		return (RegRead(SC16IS7X0_TXLVL) != 0);
	}

	bool IsReadable(void) {
		return (RegRead(SC16IS7X0_RXLVL) != 0);
	}

	bool IsReadable(uint32_t nTimeOut) {
		const uint32_t nMillis = Hardware::Get()->Millis();
		do {
			if (IsReadable()) {
				return true;
			}
		} while ((Hardware::Get()->Millis() - nTimeOut) < nMillis);

		return false;
	}

	void I2cSetup(void);
	void RegWrite(uint8_t nRegister, uint8_t nValue);
	uint8_t RegRead(uint8_t nRegister);

private:
	uint8_t m_nAddress;
	uint32_t m_nOnBoardCrystal;
};

#endif /* SC16IS740_H_ */
