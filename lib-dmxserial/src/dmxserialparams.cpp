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
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "dmxserialparams.h"
#include "dmxserialparamsconst.h"
#include "dmxserial.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "serial.h"

#include "debug.h"

DmxSerialParams::DmxSerialParams(DmxSerialParamsStore *pDmxSerialParamsStore):  m_pDmxSerialParamsStore(pDmxSerialParamsStore) {
	DEBUG_ENTRY

	DEBUG_PRINTF("sizeof(struct TDmxSerialParams) = %d", static_cast<int>(sizeof(struct TDmxSerialParams)));

	m_tDmxSerialParams.nSetList = 0;
	m_tDmxSerialParams.nType = DMXSERIAL_DEFAULT_TYPE;
	m_tDmxSerialParams.nBaud = DMXSERIAL_DEFAULT_UART_BAUD;
	m_tDmxSerialParams.nBits = DMXSERIAL_DEFAULT_UART_BITS;
	m_tDmxSerialParams.nParity = DMXSERIAL_DEFAULT_UART_PARITY;
	m_tDmxSerialParams.nStopBits = DMXSERIAL_DEFAULT_UART_STOPBITS;
	m_tDmxSerialParams.nSpiSpeedHz = DMXSERIAL_DEFAULT_SPI_SPEED_HZ;
	m_tDmxSerialParams.nSpiMode = DMXSERIAL_DEFAULT_SPI_MODE;
	m_tDmxSerialParams.nI2cAddress = DMXSERIAL_DEFAULT_I2C_ADDRESS;
	m_tDmxSerialParams.nI2cSpeedMode = DMXSERIAL_DEFAULT_I2C_SPEED_MODE;

	DEBUG_EXIT
}

DmxSerialParams::~DmxSerialParams(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

bool DmxSerialParams::Load(void) {
	m_tDmxSerialParams.nSetList = 0;

	ReadConfigFile configfile(DmxSerialParams::staticCallbackFunction, this);

	if (configfile.Read(DmxSerialParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDmxSerialParamsStore != 0) {
			m_pDmxSerialParamsStore->Update(&m_tDmxSerialParams);
		}
	} else if (m_pDmxSerialParamsStore != 0) {
		m_pDmxSerialParamsStore->Copy(&m_tDmxSerialParams);
	} else {
		return false;
	}

	return true;
}

void DmxSerialParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);

	assert(m_pDmxSerialParamsStore != 0);

	if (m_pDmxSerialParamsStore == 0) {
		return;
	}

	m_tDmxSerialParams.nSetList = 0;

	ReadConfigFile config(DmxSerialParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDmxSerialParamsStore->Update(&m_tDmxSerialParams);
}

void DmxSerialParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t nValue8;
	uint32_t nValue32;
	char aChar[16];
	uint8_t nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::TYPE, aChar, &nLength) == SSCAN_OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nType = Serial::GetType(aChar);

		if (m_tDmxSerialParams.nType != DMXSERIAL_DEFAULT_TYPE) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_TYPE;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_TYPE;
		}
		return;
	}

	/*
	 * UART
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::UART_BAUD, &nValue32) == SSCAN_OK) {
		m_tDmxSerialParams.nBaud = nValue32;

		if (m_tDmxSerialParams.nBaud != DMXSERIAL_DEFAULT_UART_BAUD) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_BAUD;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_BAUD;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_BITS, &nValue8) == SSCAN_OK) {
		m_tDmxSerialParams.nBits = nValue8;

		if (m_tDmxSerialParams.nBits != DMXSERIAL_DEFAULT_UART_BITS) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_BITS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_BITS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::UART_PARITY, aChar, &nLength) == SSCAN_OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nType = Serial::GetUartParity(aChar);

		if (m_tDmxSerialParams.nParity != DMXSERIAL_DEFAULT_UART_PARITY) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_PARTITY;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_PARTITY;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::UART_STOPBITS, &nValue8) == SSCAN_OK) {
		m_tDmxSerialParams.nStopBits = nValue8;

		if (m_tDmxSerialParams.nStopBits != DMXSERIAL_DEFAULT_UART_STOPBITS) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_STOPBITS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_STOPBITS;
		}
		return;
	}

	/*
	 * SPI
	 */

	if (Sscan::Uint32(pLine, DmxSerialParamsConst::SPI_SPEED_HZ, &nValue32) == SSCAN_OK) {
		m_tDmxSerialParams.nSpiSpeedHz = nValue32;

		if (m_tDmxSerialParams.nSpiSpeedHz != DMXSERIAL_DEFAULT_SPI_SPEED_HZ) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxSerialParamsConst::SPI_MODE, &nValue8) == SSCAN_OK) {
		m_tDmxSerialParams.nSpiMode = nValue8;

		if ((m_tDmxSerialParams.nSpiMode == DMXSERIAL_DEFAULT_SPI_MODE) || (m_tDmxSerialParams.nSpiMode > 3)) {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_SPI_MODE;
		} else {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_SPI_MODE;
		}
		return;
	}

	/*
	 * I2C
	 */

	if (Sscan::I2cAddress(pLine, DmxSerialParamsConst::I2C_ADDRESS, &nValue8) == SSCAN_OK) {
		m_tDmxSerialParams.nI2cAddress = nValue8;

		if (m_tDmxSerialParams.nI2cAddress != DMXSERIAL_DEFAULT_I2C_ADDRESS) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_I2C_ADDRESS;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_I2C_ADDRESS;
		}
		return;
	}

	nLength = sizeof(aChar) - 1;

	if (Sscan::Char(pLine, DmxSerialParamsConst::I2C_SPEED_MODE, aChar, &nLength) == SSCAN_OK) {
		aChar[nLength] = '\0';
		m_tDmxSerialParams.nI2cSpeedMode = Serial::GetI2cSpeed(aChar);

		if (m_tDmxSerialParams.nI2cSpeedMode != DMXSERIAL_DEFAULT_I2C_SPEED_MODE) {
			m_tDmxSerialParams.nSetList |= DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE;
		} else {
			m_tDmxSerialParams.nSetList &= ~DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE;
		}
		return;
	}
}

void DmxSerialParams::Builder(const struct TDmxSerialParams *pDmxSerialParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	if (pDmxSerialParams != 0) {
		memcpy(&m_tDmxSerialParams, pDmxSerialParams, sizeof(struct TDmxSerialParams));
	} else {
		m_pDmxSerialParamsStore->Copy(&m_tDmxSerialParams);
	}

	PropertiesBuilder builder(DmxSerialParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DmxSerialParamsConst::TYPE, Serial::GetType(static_cast<TSerialTypes>(m_tDmxSerialParams.nType)), isMaskSet(DMXSERIAL_PARAMS_MASK_TYPE));

	builder.AddComment("UART");
	builder.Add(DmxSerialParamsConst::UART_BAUD, m_tDmxSerialParams.nBaud, isMaskSet(DMXSERIAL_PARAMS_MASK_BAUD));
	builder.Add(DmxSerialParamsConst::UART_BITS, m_tDmxSerialParams.nBits, isMaskSet(DMXSERIAL_PARAMS_MASK_BITS));
	builder.Add(DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<TSerialUartParity>(m_tDmxSerialParams.nParity)), isMaskSet(DMXSERIAL_PARAMS_MASK_PARTITY));
	builder.Add(DmxSerialParamsConst::UART_STOPBITS, m_tDmxSerialParams.nStopBits, isMaskSet(DMXSERIAL_PARAMS_MASK_STOPBITS));

	builder.AddComment("SPI");
	builder.Add(DmxSerialParamsConst::SPI_SPEED_HZ, m_tDmxSerialParams.nSpiSpeedHz, isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ));
	builder.Add(DmxSerialParamsConst::SPI_MODE, m_tDmxSerialParams.nSpiMode, isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_MODE));

	builder.AddComment("I2C");
	builder.AddHex8(DmxSerialParamsConst::I2C_ADDRESS, m_tDmxSerialParams.nI2cAddress, isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_ADDRESS));
	builder.Add(DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeed(static_cast<TSerialI2cSpeedModes>(m_tDmxSerialParams.nI2cSpeedMode)), isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE));

	nSize = builder.GetSize();

	return;
}

void DmxSerialParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pDmxSerialParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	DEBUG_EXIT
	return;
}

void DmxSerialParams::Set(DmxSerial *pDmxSerial) {

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_TYPE)) {
		Serial::Get()->SetType(static_cast<TSerialTypes>(m_tDmxSerialParams.nType));
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_BAUD)) {
		Serial::Get()->SetUartBaud(m_tDmxSerialParams.nBaud);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_BITS)) {
		Serial::Get()->SetUartBits(m_tDmxSerialParams.nBits);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_PARTITY)) {
		Serial::Get()->SetUartParity(static_cast<TSerialUartParity>(m_tDmxSerialParams.nParity));
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_STOPBITS)) {
		Serial::Get()->SetUartStopBits(m_tDmxSerialParams.nStopBits);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ)) {
		Serial::Get()->SetSpiSpeedHz(m_tDmxSerialParams.nSpiSpeedHz);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_MODE)) {
		Serial::Get()->SetSpiMode(static_cast<TSerialSpiModes>(m_tDmxSerialParams.nSpiMode));

	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_ADDRESS)) {
		Serial::Get()->SetI2cAddress(m_tDmxSerialParams.nI2cAddress);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE)) {
		Serial::Get()->SetI2cSpeedMode(static_cast<TSerialI2cSpeedModes>(m_tDmxSerialParams.nI2cSpeedMode));
	}
}

void DmxSerialParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxSerialParamsConst::FILE_NAME);

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_TYPE)) {
		printf(" %s=%d [%s]\n", DmxSerialParamsConst::TYPE, m_tDmxSerialParams.nType, Serial::GetType(static_cast<TSerialTypes>(m_tDmxSerialParams.nType)));
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_BAUD)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BAUD, m_tDmxSerialParams.nBaud);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_BITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BITS, m_tDmxSerialParams.nBits);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_PARTITY)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<TSerialUartParity>(m_tDmxSerialParams.nParity)), m_tDmxSerialParams.nParity);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_STOPBITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_STOPBITS, m_tDmxSerialParams.nStopBits);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_SPEED_HZ, m_tDmxSerialParams.nSpiSpeedHz);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_SPI_MODE)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_MODE, m_tDmxSerialParams.nSpiMode);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_ADDRESS)) {
		printf(" %s=%.2x\n", DmxSerialParamsConst::I2C_ADDRESS, m_tDmxSerialParams.nI2cAddress);
	}

	if (isMaskSet(DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeed(static_cast<TSerialI2cSpeedModes>(m_tDmxSerialParams.nI2cSpeedMode)), m_tDmxSerialParams.nI2cSpeedMode);
	}
#endif
}

void DmxSerialParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<DmxSerialParams *>(p))->callbackFunction(s);
}
