/**
 * @file dmxserialparamsdump.cpp
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

#include <stdio.h>

#include "dmxserialparams.h"
#include "dmxserialparamsconst.h"

using namespace serial;

void DmxSerialParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxSerialParamsConst::FILE_NAME);

	if (isMaskSet(DmxSerialParamsMask::TYPE)) {
		printf(" %s=%d [%s]\n", DmxSerialParamsConst::TYPE, m_tDmxSerialParams.nType, Serial::GetType(static_cast<type>(m_tDmxSerialParams.nType)));
	}

	if (isMaskSet(DmxSerialParamsMask::BAUD)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BAUD, m_tDmxSerialParams.nBaud);
	}

	if (isMaskSet(DmxSerialParamsMask::BITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_BITS, m_tDmxSerialParams.nBits);
	}

	if (isMaskSet(DmxSerialParamsMask::PARTITY)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::UART_PARITY, Serial::GetUartParity(static_cast<uart::parity>(m_tDmxSerialParams.nParity)), m_tDmxSerialParams.nParity);
	}

	if (isMaskSet(DmxSerialParamsMask::STOPBITS)) {
		printf(" %s=%d\n", DmxSerialParamsConst::UART_STOPBITS, m_tDmxSerialParams.nStopBits);
	}

	if (isMaskSet(DmxSerialParamsMask::SPI_SPEED_HZ)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_SPEED_HZ, m_tDmxSerialParams.nSpiSpeedHz);
	}

	if (isMaskSet(DmxSerialParamsMask::SPI_MODE)) {
		printf(" %s=%d\n", DmxSerialParamsConst::SPI_MODE, m_tDmxSerialParams.nSpiMode);
	}

	if (isMaskSet(DmxSerialParamsMask::I2C_ADDRESS)) {
		printf(" %s=%.2x\n", DmxSerialParamsConst::I2C_ADDRESS, m_tDmxSerialParams.nI2cAddress);
	}

	if (isMaskSet(DmxSerialParamsMask::I2C_SPEED_MODE)) {
		printf(" %s=%s [%d]\n", DmxSerialParamsConst::I2C_SPEED_MODE, Serial::GetI2cSpeed(static_cast<i2c::speed>(m_tDmxSerialParams.nI2cSpeedMode)), m_tDmxSerialParams.nI2cSpeedMode);
	}
#endif
}
