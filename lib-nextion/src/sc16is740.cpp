/**
 * @file sc16is750.cpp
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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "sc16is740.h"
#include "sc16is7x0.h"

#include "i2c.h"

#include "debug.h"

SC16IS740::SC16IS740(void) {
	m_nAddress = SC16IS740_I2C_ADDRESS;
	m_nOnBoardCrystal = SC16IS740_CRISTAL_HZ;
}

SC16IS740::SC16IS740(uint8_t nAddress, uint32_t nOnBoardCrystal):
	m_nAddress(nAddress),
	m_nOnBoardCrystal(nOnBoardCrystal)
{
}

SC16IS740::~SC16IS740(void) {
}

bool SC16IS740::Init(void) {
	i2c_begin();

	I2cSetup();

	if (!i2c_is_connected(m_nAddress)) {
		DEBUG_PRINTF("I2C: Not connected at address %.2x", m_nAddress);
		return false;
	}

	SetFormat(8, SC16IS740_SERIAL_PARITY_NONE, 1);
	SetBaud(SC16IS7X0_DEFAULT_BAUDRATE);

	const uint8_t nTestCharacter = 'A';
	RegWrite(SC16IS7X0_SPR, nTestCharacter);

	if ((RegRead(SC16IS7X0_SPR) != nTestCharacter)) {
		return false;
	}

	//
	uint32_t nRegisterMCR = RegRead(SC16IS7X0_MCR);
	nRegisterMCR |= MCR_ENABLE_TCR_TLR;
	RegWrite(SC16IS7X0_MCR, nRegisterMCR);

	uint32_t nRegisterEFR = RegRead(SC16IS7X0_EFR);
	RegWrite(SC16IS7X0_EFR, nRegisterEFR | EFR_ENABLE_ENHANCED_FUNCTIONS);

	RegWrite(SC16IS7X0_TLR, 0x10);

	RegWrite(SC16IS7X0_EFR, nRegisterEFR);
	//

	RegWrite(SC16IS7X0_FCR, (FCR_RX_FIFO_RST | FCR_TX_FIFO_RST));
	RegWrite(SC16IS7X0_FCR, FCR_ENABLE_FIFO);
	RegWrite(SC16IS7X0_IER, IER_ELSI | IER_ERHRI);

	DEBUG_PRINTF("TLR=%.2x", RegRead(SC16IS7X0_TLR));
	debug_print_bits(RegRead(SC16IS7X0_TLR));

	DEBUG_PRINTF("IER=%.2x", RegRead(SC16IS7X0_IER));
	debug_print_bits(RegRead(SC16IS7X0_IER));

	DEBUG_PRINTF("IIR=%.2x", RegRead(SC16IS7X0_IIR));
	debug_print_bits(RegRead(SC16IS7X0_IIR));

	return true;
}

void SC16IS740::SetFormat(uint32_t nBits, TSC16IS740SerialParity tParity,  uint32_t nStopBits) {
	uint8_t nRegisterLCR = 0x00;

	switch (nBits) {
	case 5:
		nRegisterLCR |= LCR_BITS5;
		break;
	case 6:
		nRegisterLCR |= LCR_BITS6;
		break;
	case 7:
		nRegisterLCR |= LCR_BITS7;
		break;
	case 8:
		nRegisterLCR |= LCR_BITS8;
		break;
	default:
		nRegisterLCR |= LCR_BITS8;
	}

	switch (tParity) {
	case SERIAL_PARITY_NONE:
		nRegisterLCR |= LCR_NONE;
		break;
	case SERIAL_PARITY_ODD:
		nRegisterLCR |= LCR_ODD;
		break;
	case SERIAL_PARITY_EVEN:
		nRegisterLCR |= LCR_EVEN;
		break;
	case SERIAL_PARITY_FORCED1:
		nRegisterLCR |= LCR_FORCED1;
		break;
	case SERIAL_PARITY_FORCED0:
		nRegisterLCR |= LCR_FORCED0;
		break;
	default:
		nRegisterLCR |= LCR_NONE;
	}

	switch (nStopBits) {
	case 1:
		nRegisterLCR |= LCR_BITS1;
		break;
	case 2:
		nRegisterLCR |= LCR_BITS2;
		break;
	default:
		nRegisterLCR |= LCR_BITS1;
	}

	RegWrite(SC16IS7X0_LCR, nRegisterLCR);
}

void SC16IS740::SetBaud(uint32_t nBaud) {
	uint32_t nPrescaler;

	if ((RegRead(SC16IS7X0_MCR) & MCR_PRESCALE_4) == MCR_PRESCALE_4) {
		nPrescaler = 4;
	} else {
		nPrescaler = 1;
	}

	const uint32_t nDivisor = ((m_nOnBoardCrystal / nPrescaler) / (nBaud * 16));
	const uint8_t nRegisterLCR = RegRead(SC16IS7X0_LCR);

	RegWrite(SC16IS7X0_LCR, nRegisterLCR | LCR_ENABLE_DIV);
	RegWrite(SC16IS7X0_DLL, (nDivisor & 0xFF));
	RegWrite(SC16IS7X0_DLH, ((nDivisor >> 8) & 0xFF));
	RegWrite(SC16IS7X0_LCR, nRegisterLCR);

	DEBUG_PRINTF("nPrescaler=%u", nPrescaler);
	DEBUG_PRINTF("nDivisor=%u", nDivisor);
	DEBUG_PRINTF("m_nOnBoardCrystal=%u", m_nOnBoardCrystal);
}

void SC16IS740::WriteBytes(const uint8_t *pBytes, uint32_t nSize) {
	uint8_t *p = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(pBytes));

	while (nSize > 0) {
		uint32_t nAvailable = RegRead(SC16IS7X0_TXLVL);

		// DEBUG_PRINTF("nAvailable=%d", nAvailable);

		while ((nSize > 0) && (nAvailable > 0)) {
			RegWrite(SC16IS7X0_THR, *p);
			nSize--;
			nAvailable--;
			p++;
		}
	}
}

void SC16IS740::ReadBytes(uint8_t *pBytes, uint32_t &nSize, uint32_t nTimeOut) {
	uint8_t *Destination = pBytes;
	uint32_t nRemaining = nSize;

	while (nRemaining > 0) {
		const uint32_t nMillis = Hardware::Get()->Millis();
		uint32_t nAvailable;

		while ((nAvailable = RegRead(SC16IS7X0_RXLVL)) == 0) {
			if ((Hardware::Get()->Millis() - nTimeOut) > nMillis) {
				nRemaining = 0;
				break;
			}
		}

		// DEBUG_PRINTF("nAvailable=%d", nAvailable);

		while ((nRemaining > 0) && (nAvailable > 0)) {
			*Destination++ = RegRead(SC16IS7X0_RHR);
			nRemaining--;
			nAvailable--;
		}
	}

	nSize = (Destination - pBytes);
}

void SC16IS740::FlushRead(uint32_t nTimeOut) {
	bool bIsRemaining = true;

	while (bIsRemaining) {
		const uint32_t nMillis = Hardware::Get()->Millis();
		uint32_t nAvailable;

		while ((nAvailable = RegRead(SC16IS7X0_RXLVL)) == 0) {
			if ((Hardware::Get()->Millis() - nTimeOut) > nMillis) {
				bIsRemaining = false;
				break;
			}
		}

		// DEBUG_PRINTF("nAvailable=%d", nAvailable);

		while (nAvailable > 0) {
			RegRead(SC16IS7X0_RHR);
			nAvailable--;
		}
	}
}

// Private

void SC16IS740::RegWrite(uint8_t nRegister, uint8_t nValue) {
	I2cSetup();
	i2c_write_reg_uint8(nRegister << 3, nValue);
}

uint8_t SC16IS740::RegRead(uint8_t nRegister) {
	I2cSetup();
	return i2c_read_reg_uint8(nRegister << 3);
}

void SC16IS740::I2cSetup(void) {
	i2c_set_baudrate(I2C_FULL_SPEED);
	i2c_set_address(m_nAddress);
}
