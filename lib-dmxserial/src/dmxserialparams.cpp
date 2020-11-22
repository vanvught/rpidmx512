/**
 * @file dmxserialparams.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "dmxserialparams.h"
#include "dmxserialparamsconst.h"
#include "dmxserial.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "../src/serial/serial.h"

#include "debug.h"

using namespace serial;

DmxSerialParams::DmxSerialParams(DmxSerialParamsStore *pDmxSerialParamsStore):  m_pDmxSerialParamsStore(pDmxSerialParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct TDmxSerialParams) = %d", static_cast<int>(sizeof(struct TDmxSerialParams)));

	m_tDmxSerialParams.nSetList = 0;
	m_tDmxSerialParams.nType = static_cast<uint8_t>(DmxSerialDefaults::TYPE);
	m_tDmxSerialParams.nBaud = DmxSerialDefaults::UART_BAUD;
	m_tDmxSerialParams.nBits = DmxSerialDefaults::UART_BITS;
	m_tDmxSerialParams.nParity = static_cast<uint8_t>(DmxSerialDefaults::UART_PARITY);
	m_tDmxSerialParams.nStopBits = DmxSerialDefaults::UART_STOPBITS;
	m_tDmxSerialParams.nSpiSpeedHz = DmxSerialDefaults::SPI_SPEED_HZ;
	m_tDmxSerialParams.nSpiMode = DmxSerialDefaults::SPI_MODE;
	m_tDmxSerialParams.nI2cAddress = DmxSerialDefaults::I2C_ADDRESS;
	m_tDmxSerialParams.nI2cSpeedMode = static_cast<uint8_t>(DmxSerialDefaults::I2C_SPEED_MODE);

	DEBUG_EXIT
}

bool DmxSerialParams::Load() {
	m_tDmxSerialParams.nSetList = 0;

	ReadConfigFile configfile(DmxSerialParams::staticCallbackFunction, this);

	if (configfile.Read(DmxSerialParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDmxSerialParamsStore != nullptr) {
			m_pDmxSerialParamsStore->Update(&m_tDmxSerialParams);
		}
	} else if (m_pDmxSerialParamsStore != nullptr) {
		m_pDmxSerialParamsStore->Copy(&m_tDmxSerialParams);
	} else {
		return false;
	}

	return true;
}

void DmxSerialParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pDmxSerialParamsStore != nullptr);

	if (m_pDmxSerialParamsStore == nullptr) {
		return;
	}

	m_tDmxSerialParams.nSetList = 0;

	ReadConfigFile config(DmxSerialParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDmxSerialParamsStore->Update(&m_tDmxSerialParams);
}

void DmxSerialParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint32_t nValue32;
	char aChar[16];
	uint32_t nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::TYPE, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nType = static_cast<uint8_t>(Serial::GetType(aChar));

		if (m_tDmxSerialParams.nType != static_cast<uint8_t>(DmxSerialDefaults::TYPE)) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::TYPE;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::TYPE;
		}
		return;
	}

	/*
	 * UART
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::UART_BAUD, nValue32) == Sscan::OK) {
		m_tDmxSerialParams.nBaud = nValue32;

		if (m_tDmxSerialParams.nBaud != DmxSerialDefaults::UART_BAUD) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::BAUD;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::BAUD;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_BITS, nValue8) == Sscan::OK) {
		m_tDmxSerialParams.nBits = nValue8;

		if (m_tDmxSerialParams.nBits != DmxSerialDefaults::UART_BITS) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::BITS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::BITS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::UART_PARITY, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nParity = static_cast<uint8_t>(Serial::GetUartParity(aChar));

		if (m_tDmxSerialParams.nParity != static_cast<uint8_t>(DmxSerialDefaults::UART_PARITY)) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::PARTITY;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::PARTITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_STOPBITS, nValue8) == Sscan::OK) {
		m_tDmxSerialParams.nStopBits = nValue8;

		if (m_tDmxSerialParams.nStopBits != DmxSerialDefaults::UART_STOPBITS) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::STOPBITS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::STOPBITS;
		}
		return;
	}

	/*
	 * SPI
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::SPI_SPEED_HZ, nValue32) == Sscan::OK) {
		m_tDmxSerialParams.nSpiSpeedHz = nValue32;

		if (m_tDmxSerialParams.nSpiSpeedHz != DmxSerialDefaults::SPI_SPEED_HZ) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::SPI_SPEED_HZ;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::SPI_SPEED_HZ;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::SPI_MODE, nValue8) == Sscan::OK) {
		m_tDmxSerialParams.nSpiMode = nValue8;

		if ((m_tDmxSerialParams.nSpiMode == DmxSerialDefaults::SPI_MODE) || (m_tDmxSerialParams.nSpiMode > 3)) {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::SPI_MODE;
		} else {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::SPI_MODE;
		}
		return;
	}

	/*
	 * I2C
	 */

	if (Sscan::I2cAddress(pLine, DmxSerialParamsConst::I2C_ADDRESS, nValue8) == Sscan::OK) {
		m_tDmxSerialParams.nI2cAddress = nValue8;

		if (m_tDmxSerialParams.nI2cAddress != DmxSerialDefaults::I2C_ADDRESS) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::I2C_ADDRESS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::I2C_ADDRESS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::I2C_SPEED_MODE, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nI2cSpeedMode = static_cast<uint8_t>(Serial::GetI2cSpeed(aChar));

		if (m_tDmxSerialParams.nI2cSpeedMode != static_cast<uint8_t>(DmxSerialDefaults::I2C_SPEED_MODE)) {
			m_tDmxSerialParams.nSetList |= DmxSerialParamsMask::I2C_SPEED_MODE;
		} else {
			m_tDmxSerialParams.nSetList &= ~DmxSerialParamsMask::I2C_SPEED_MODE;
		}
		return;
	}
}

void DmxSerialParams::Builder(const struct TDmxSerialParams *pDmxSerialParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (pDmxSerialParams != nullptr) {
		memcpy(&m_tDmxSerialParams, pDmxSerialParams, sizeof(struct TDmxSerialParams));
	} else {
		m_pDmxSerialParamsStore->Copy(&m_tDmxSerialParams);
	}

	PropertiesBuilder builder(DmxSerialParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DmxSerialParamsConst::TYPE, Serial::GetType(static_cast<type>(m_tDmxSerialParams.nType)), isMaskSet(DmxSerialParamsMask::TYPE));

	builder.AddComment("UART");
	builder.Add(DmxSerialParamsConst::UART_BAUD, m_tDmxSerialParams.nBaud, isMaskSet(DmxSerialParamsMask::BAUD));
	builder.Add(DmxSerialParamsConst::UART_BITS, m_tDmxSerialParams.nBits, isMaskSet(DmxSerialParamsMask::BITS));
	builder.Add(DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<uart::parity>(m_tDmxSerialParams.nParity)), isMaskSet(DmxSerialParamsMask::PARTITY));
	builder.Add(DmxSerialParamsConst::UART_STOPBITS, m_tDmxSerialParams.nStopBits, isMaskSet(DmxSerialParamsMask::STOPBITS));

	builder.AddComment("SPI");
	builder.Add(DmxSerialParamsConst::SPI_SPEED_HZ, m_tDmxSerialParams.nSpiSpeedHz, isMaskSet(DmxSerialParamsMask::SPI_SPEED_HZ));
	builder.Add(DmxSerialParamsConst::SPI_MODE, m_tDmxSerialParams.nSpiMode, isMaskSet(DmxSerialParamsMask::SPI_MODE));

	builder.AddComment("I2C");
	builder.AddHex8(DmxSerialParamsConst::I2C_ADDRESS, m_tDmxSerialParams.nI2cAddress, isMaskSet(DmxSerialParamsMask::I2C_ADDRESS));
	builder.Add(DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeed(static_cast<i2c::speed>(m_tDmxSerialParams.nI2cSpeedMode)), isMaskSet(DmxSerialParamsMask::I2C_SPEED_MODE));

	nSize = builder.GetSize();
}

void DmxSerialParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pDmxSerialParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}

void DmxSerialParams::Set() {
	if (isMaskSet(DmxSerialParamsMask::TYPE)) {
		Serial::Get()->SetType(static_cast<type>(m_tDmxSerialParams.nType));
	}

	if (isMaskSet(DmxSerialParamsMask::BAUD)) {
		Serial::Get()->SetUartBaud(m_tDmxSerialParams.nBaud);
	}

	if (isMaskSet(DmxSerialParamsMask::BITS)) {
		Serial::Get()->SetUartBits(m_tDmxSerialParams.nBits);
	}

	if (isMaskSet(DmxSerialParamsMask::PARTITY)) {
		Serial::Get()->SetUartParity(static_cast<uart::parity>(m_tDmxSerialParams.nParity));
	}

	if (isMaskSet(DmxSerialParamsMask::STOPBITS)) {
		Serial::Get()->SetUartStopBits(m_tDmxSerialParams.nStopBits);
	}

	if (isMaskSet(DmxSerialParamsMask::SPI_SPEED_HZ)) {
		Serial::Get()->SetSpiSpeedHz(m_tDmxSerialParams.nSpiSpeedHz);
	}

	if (isMaskSet(DmxSerialParamsMask::SPI_MODE)) {
		Serial::Get()->SetSpiMode(static_cast<spi::mode>(m_tDmxSerialParams.nSpiMode));
	}

	if (isMaskSet(DmxSerialParamsMask::I2C_ADDRESS)) {
		Serial::Get()->SetI2cAddress(m_tDmxSerialParams.nI2cAddress);
	}

	if (isMaskSet(DmxSerialParamsMask::I2C_SPEED_MODE)) {
		Serial::Get()->SetI2cSpeedMode(static_cast<i2c::speed>(m_tDmxSerialParams.nI2cSpeedMode));
	}
}

void DmxSerialParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxSerialParams *>(p))->callbackFunction(s);
}
