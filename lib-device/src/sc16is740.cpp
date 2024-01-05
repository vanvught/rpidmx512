/**
 * @file sc16is740.cpp
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

#include <cstdint>
#include <cassert>

#include "sc16is740.h"
#include "sc16is7x0.h"

#include "debug.h"

SC16IS740::SC16IS740(uint8_t nAddress, uint32_t nOnBoardCrystal) :
	HAL_I2C(nAddress, hal::i2c::FULL_SPEED),
	m_nOnBoardCrystal(nOnBoardCrystal)
{
	m_IsConnected = IsConnected();

	if (!m_IsConnected) {
		DEBUG_PRINTF("Not connected at address %.2x", GetAddress());
		return;
	}

	SetFormat(8, SerialParity::NONE, 1);
	SetBaud(SC16IS7X0_DEFAULT_BAUDRATE);

	const uint8_t nTestCharacter = 'A';
	WriteRegister(SC16IS7X0_SPR, nTestCharacter);

	if ((ReadRegister(SC16IS7X0_SPR) != nTestCharacter)) {
		DEBUG_PUTS("Test character failed");
		m_IsConnected = false;
		return ;
	}

	auto nRegisterMCR = ReadRegister(SC16IS7X0_MCR);
	nRegisterMCR |= MCR_ENABLE_TCR_TLR;
	WriteRegister(SC16IS7X0_MCR, nRegisterMCR);

	auto nRegisterEFR = ReadRegister(SC16IS7X0_EFR);
	WriteRegister(SC16IS7X0_EFR, static_cast<uint8_t>(nRegisterEFR | EFR_ENABLE_ENHANCED_FUNCTIONS));

	WriteRegister(SC16IS7X0_TLR, static_cast<uint8_t>(0x10));

	WriteRegister(SC16IS7X0_EFR, nRegisterEFR);

	WriteRegister(SC16IS7X0_FCR, static_cast<uint8_t>(FCR_RX_FIFO_RST | FCR_TX_FIFO_RST));
	WriteRegister(SC16IS7X0_FCR, FCR_ENABLE_FIFO);
	WriteRegister(SC16IS7X0_IER, static_cast<uint8_t>(IER_ELSI | IER_ERHRI));

	DEBUG_PRINTF("TLR=%.2x", ReadRegister(SC16IS7X0_TLR));
	debug_print_bits(ReadRegister(SC16IS7X0_TLR));

	DEBUG_PRINTF("IER=%.2x", ReadRegister(SC16IS7X0_IER));
	debug_print_bits(ReadRegister(SC16IS7X0_IER));

	DEBUG_PRINTF("IIR=%.2x", ReadRegister(SC16IS7X0_IIR));
	debug_print_bits(ReadRegister(SC16IS7X0_IIR));
}

void SC16IS740::SetFormat(uint32_t nBits, SerialParity tParity,  uint32_t nStopBits) {
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
	case SerialParity::NONE:
		nRegisterLCR |= LCR_NONE;
		break;
	case SerialParity::ODD:
		nRegisterLCR |= LCR_ODD;
		break;
	case SerialParity::EVEN:
		nRegisterLCR |= LCR_EVEN;
		break;
	case SerialParity::FORCED1:
		nRegisterLCR |= LCR_FORCED1;
		break;
	case SerialParity::FORCED0:
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

	WriteRegister(SC16IS7X0_LCR, nRegisterLCR);

	DEBUG_PRINTF("LCR=%.2x:%.2x", ReadRegister(SC16IS7X0_LCR), nRegisterLCR);
}

void SC16IS740::SetBaud(uint32_t nBaud) {
	uint32_t nPrescaler;

	if ((ReadRegister(SC16IS7X0_MCR) & MCR_PRESCALE_4) == MCR_PRESCALE_4) {
		nPrescaler = 4;
	} else {
		nPrescaler = 1;
	}

	const uint32_t nDivisor = ((m_nOnBoardCrystal / nPrescaler) / (nBaud * 16));
	const auto nRegisterLCR = ReadRegister(SC16IS7X0_LCR);

	WriteRegister(SC16IS7X0_LCR, static_cast<uint8_t>(nRegisterLCR | LCR_ENABLE_DIV));
	WriteRegister(SC16IS7X0_DLL, static_cast<uint8_t>(nDivisor & 0xFF));
	WriteRegister(SC16IS7X0_DLH, static_cast<uint8_t>((nDivisor >> 8) & 0xFF));
	WriteRegister(SC16IS7X0_LCR, nRegisterLCR);

	DEBUG_PRINTF("nPrescaler=%u", nPrescaler);
	DEBUG_PRINTF("nDivisor=%u", nDivisor);
	DEBUG_PRINTF("m_nOnBoardCrystal=%u", m_nOnBoardCrystal);

	DEBUG_PRINTF("LCR=%.2x:%.2x", ReadRegister(SC16IS7X0_LCR), nRegisterLCR);
}

void SC16IS740::WriteBytes(const uint8_t *pBytes, uint32_t nSize) {
	if (!m_IsConnected) {
		return;
	}

	auto *p = const_cast<uint8_t *>(pBytes);

	while (nSize > 0) {
		uint32_t nAvailable = ReadRegister(SC16IS7X0_TXLVL);

		while ((nSize > 0) && (nAvailable > 0)) {
			WriteRegister(SC16IS7X0_THR, *p);
			nSize--;
			nAvailable--;
			p++;
		}
	}
}

void SC16IS740::ReadBytes(uint8_t *pBytes, uint32_t& nSize, uint32_t nTimeOut) {
	if (!m_IsConnected) {
		nSize = 0;
		return;
	}

	auto *Destination = pBytes;
	uint32_t nRemaining = nSize;

	while (nRemaining > 0) {
		const uint32_t nMillis = Hardware::Get()->Millis();
		uint32_t nAvailable;

		while ((nAvailable = ReadRegister(SC16IS7X0_RXLVL)) == 0) {
			if ((Hardware::Get()->Millis() - nTimeOut) > nMillis) {
				nRemaining = 0;
				break;
			}
		}

		while ((nRemaining > 0) && (nAvailable > 0)) {
			*Destination++ = ReadRegister(SC16IS7X0_RHR);
			nRemaining--;
			nAvailable--;
		}
	}

	nSize = static_cast<uint16_t>(Destination - pBytes);
}

void SC16IS740::FlushRead(uint32_t nTimeOut) {
	if (!m_IsConnected) {
		return;
	}

	bool bIsRemaining = true;

	while (bIsRemaining) {
		const auto nMillis = Hardware::Get()->Millis();
		uint32_t nAvailable;

		while ((nAvailable = ReadRegister(SC16IS7X0_RXLVL)) == 0) {
			if ((Hardware::Get()->Millis() - nTimeOut) > nMillis) {
				bIsRemaining = false;
				break;
			}
		}

		while (nAvailable > 0) {
			ReadRegister(SC16IS7X0_RHR);
			nAvailable--;
		}
	}
}
