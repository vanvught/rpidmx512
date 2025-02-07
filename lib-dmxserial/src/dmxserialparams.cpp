/**
 * @file dmxserialparams.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
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

DmxSerialParams::DmxSerialParams() {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct dmxserialparams::Params) = %d", static_cast<int>(sizeof(struct dmxserialparams::Params)));

	m_Params.nSetList = 0;
	m_Params.nType = static_cast<uint8_t>(DmxSerialDefaults::TYPE);
	m_Params.nBaud = DmxSerialDefaults::UART_BAUD;
	m_Params.nBits = DmxSerialDefaults::UART_BITS;
	m_Params.nParity = static_cast<uint8_t>(DmxSerialDefaults::UART_PARITY);
	m_Params.nStopBits = DmxSerialDefaults::UART_STOPBITS;
	m_Params.nSpiSpeedHz = DmxSerialDefaults::SPI_SPEED_HZ;
	m_Params.nSpiMode = DmxSerialDefaults::SPI_MODE;
	m_Params.nI2cAddress = DmxSerialDefaults::I2C_ADDRESS;
	m_Params.nI2cSpeedMode = static_cast<uint8_t>(DmxSerialDefaults::I2C_SPEED_MODE);

	DEBUG_EXIT
}

void DmxSerialParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DmxSerialParams::StaticCallbackFunction, this);

	if (configfile.Read(DmxSerialParamsConst::FILE_NAME)) {
		DmxSerialStore::Update(&m_Params);
	} else
#endif
		DmxSerialStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DmxSerialParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(DmxSerialParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	DmxSerialStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DmxSerialParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint32_t nValue32;
	char aChar[16];
	uint32_t nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::TYPE, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_Params.nType = static_cast<uint8_t>(Serial::GetType(aChar));

		if (m_Params.nType != static_cast<uint8_t>(DmxSerialDefaults::TYPE)) {
			m_Params.nSetList |= dmxserialparams::Mask::TYPE;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::TYPE;
		}
		return;
	}

	/*
	 * UART
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::UART_BAUD, nValue32) == Sscan::OK) {
		m_Params.nBaud = nValue32;

		if (m_Params.nBaud != DmxSerialDefaults::UART_BAUD) {
			m_Params.nSetList |= dmxserialparams::Mask::BAUD;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::BAUD;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_BITS, nValue8) == Sscan::OK) {
		m_Params.nBits = nValue8;

		if (m_Params.nBits != DmxSerialDefaults::UART_BITS) {
			m_Params.nSetList |= dmxserialparams::Mask::BITS;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::BITS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::UART_PARITY, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_Params.nParity = static_cast<uint8_t>(Serial::GetUartParity(aChar));

		if (m_Params.nParity != static_cast<uint8_t>(DmxSerialDefaults::UART_PARITY)) {
			m_Params.nSetList |= dmxserialparams::Mask::PARTITY;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::PARTITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_STOPBITS, nValue8) == Sscan::OK) {
		m_Params.nStopBits = nValue8;

		if (m_Params.nStopBits != DmxSerialDefaults::UART_STOPBITS) {
			m_Params.nSetList |= dmxserialparams::Mask::STOPBITS;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::STOPBITS;
		}
		return;
	}

	/*
	 * SPI
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::SPI_SPEED_HZ, nValue32) == Sscan::OK) {
		m_Params.nSpiSpeedHz = nValue32;

		if (m_Params.nSpiSpeedHz != DmxSerialDefaults::SPI_SPEED_HZ) {
			m_Params.nSetList |= dmxserialparams::Mask::SPI_SPEED_HZ;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::SPI_SPEED_HZ;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::SPI_MODE, nValue8) == Sscan::OK) {
		m_Params.nSpiMode = nValue8;

		if ((m_Params.nSpiMode == DmxSerialDefaults::SPI_MODE) || (m_Params.nSpiMode > 3)) {
			m_Params.nSetList &= ~dmxserialparams::Mask::SPI_MODE;
		} else {
			m_Params.nSetList |= dmxserialparams::Mask::SPI_MODE;
		}
		return;
	}

	/*
	 * I2C
	 */

	if (Sscan::I2cAddress(pLine, DmxSerialParamsConst::I2C_ADDRESS, nValue8) == Sscan::OK) {
		m_Params.nI2cAddress = nValue8;

		if (m_Params.nI2cAddress != DmxSerialDefaults::I2C_ADDRESS) {
			m_Params.nSetList |= dmxserialparams::Mask::I2C_ADDRESS;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::I2C_ADDRESS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::I2C_SPEED_MODE, aChar, nLength) == Sscan::OK) {
		aChar[nLength] = '\0';
		m_Params.nI2cSpeedMode = static_cast<uint8_t>(Serial::GetI2cSpeedMode(aChar));

		if (m_Params.nI2cSpeedMode != static_cast<uint8_t>(DmxSerialDefaults::I2C_SPEED_MODE)) {
			m_Params.nSetList |= dmxserialparams::Mask::I2C_SPEED_MODE;
		} else {
			m_Params.nSetList &= ~dmxserialparams::Mask::I2C_SPEED_MODE;
		}
		return;
	}
}

void DmxSerialParams::Builder(const struct dmxserialparams::Params *pDmxSerialParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (pDmxSerialParams != nullptr) {
		memcpy(&m_Params, pDmxSerialParams, sizeof(struct dmxserialparams::Params));
	} else {
		DmxSerialStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(DmxSerialParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DmxSerialParamsConst::TYPE, Serial::GetType(static_cast<type>(m_Params.nType)), isMaskSet(dmxserialparams::Mask::TYPE));

	builder.AddComment("UART");
	builder.Add(DmxSerialParamsConst::UART_BAUD, m_Params.nBaud, isMaskSet(dmxserialparams::Mask::BAUD));
	builder.Add(DmxSerialParamsConst::UART_BITS, m_Params.nBits, isMaskSet(dmxserialparams::Mask::BITS));
	builder.Add(DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<uart::parity>(m_Params.nParity)), isMaskSet(dmxserialparams::Mask::PARTITY));
	builder.Add(DmxSerialParamsConst::UART_STOPBITS, m_Params.nStopBits, isMaskSet(dmxserialparams::Mask::STOPBITS));

	builder.AddComment("SPI");
	builder.Add(DmxSerialParamsConst::SPI_SPEED_HZ, m_Params.nSpiSpeedHz, isMaskSet(dmxserialparams::Mask::SPI_SPEED_HZ));
	builder.Add(DmxSerialParamsConst::SPI_MODE, m_Params.nSpiMode, isMaskSet(dmxserialparams::Mask::SPI_MODE));

	builder.AddComment("I2C");
	builder.AddHex8(DmxSerialParamsConst::I2C_ADDRESS, m_Params.nI2cAddress, isMaskSet(dmxserialparams::Mask::I2C_ADDRESS));
	builder.Add(DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeedMode(static_cast<i2c::speed>(m_Params.nI2cSpeedMode)), isMaskSet(dmxserialparams::Mask::I2C_SPEED_MODE));

	nSize = builder.GetSize();
}

void DmxSerialParams::Set() {
	if (isMaskSet(dmxserialparams::Mask::TYPE)) {
		Serial::Get()->SetType(static_cast<type>(m_Params.nType));
	}

	if (isMaskSet(dmxserialparams::Mask::BAUD)) {
		Serial::Get()->SetUartBaud(m_Params.nBaud);
	}

	if (isMaskSet(dmxserialparams::Mask::BITS)) {
		Serial::Get()->SetUartBits(m_Params.nBits);
	}

	if (isMaskSet(dmxserialparams::Mask::PARTITY)) {
		Serial::Get()->SetUartParity(static_cast<uart::parity>(m_Params.nParity));
	}

	if (isMaskSet(dmxserialparams::Mask::STOPBITS)) {
		Serial::Get()->SetUartStopBits(m_Params.nStopBits);
	}

	if (isMaskSet(dmxserialparams::Mask::SPI_SPEED_HZ)) {
		Serial::Get()->SetSpiSpeedHz(m_Params.nSpiSpeedHz);
	}

	if (isMaskSet(dmxserialparams::Mask::SPI_MODE)) {
		Serial::Get()->SetSpiMode(m_Params.nSpiMode);
	}

	if (isMaskSet(dmxserialparams::Mask::I2C_ADDRESS)) {
		Serial::Get()->SetI2cAddress(m_Params.nI2cAddress);
	}

	if (isMaskSet(dmxserialparams::Mask::I2C_SPEED_MODE)) {
		Serial::Get()->SetI2cSpeedMode(static_cast<i2c::speed>(m_Params.nI2cSpeedMode));
	}
}

void DmxSerialParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxSerialParams *>(p))->callbackFunction(s);
}

void DmxSerialParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxSerialParamsConst::FILE_NAME);

	if (isMaskSet(dmxserialparams::Mask::TYPE)) {
		printf(" %s=%d [%s]\n", DmxSerialParamsConst::TYPE, m_Params.nType, Serial::GetType(static_cast<type>(m_Params.nType)));
	}

	if (isMaskSet(dmxserialparams::Mask::BAUD)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BAUD, m_Params.nBaud);
	}

	if (isMaskSet(dmxserialparams::Mask::BITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BITS, m_Params.nBits);
	}

	if (isMaskSet(dmxserialparams::Mask::PARTITY)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<uart::parity>(m_Params.nParity)), m_Params.nParity);
	}

	if (isMaskSet(dmxserialparams::Mask::STOPBITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_STOPBITS, m_Params.nStopBits);
	}

	if (isMaskSet(dmxserialparams::Mask::SPI_SPEED_HZ)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_SPEED_HZ, m_Params.nSpiSpeedHz);
	}

	if (isMaskSet(dmxserialparams::Mask::SPI_MODE)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_MODE, m_Params.nSpiMode);
	}

	if (isMaskSet(dmxserialparams::Mask::I2C_ADDRESS)) {
		printf(" %s=%.2x\n", DmxSerialParamsConst::I2C_ADDRESS, m_Params.nI2cAddress);
	}

	if (isMaskSet(dmxserialparams::Mask::I2C_SPEED_MODE)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeedMode(static_cast<i2c::speed>(m_Params.nI2cSpeedMode)), m_Params.nI2cSpeedMode);
	}
}
