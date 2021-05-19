/**
 * @file serialspi.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "serial.h"

#include "hal_spi.h"

#include "debug.h"

using namespace serial;

void Serial::SetSpiSpeedHz(uint32_t nSpeedHz) {
	DEBUG_PRINTF("nSpeedHz=%d", nSpeedHz);

	if (nSpeedHz == 0) {
		return;

	}

	m_SpiConfiguration.nSpeed = nSpeedHz;
}

void Serial::SetSpiMode(uint32_t nMode) {
	DEBUG_PRINTF("tMode=%d", nMode);

	if (nMode > 3) {
		return;
	}

	m_SpiConfiguration.nMode = static_cast<uint8_t>(nMode);
}

bool Serial::InitSpi() {
	DEBUG_ENTRY

	FUNC_PREFIX (spi_begin());
	FUNC_PREFIX (spi_set_speed_hz(m_SpiConfiguration.nSpeed));
	FUNC_PREFIX (spi_chipSelect(SPI_CS0));
	FUNC_PREFIX (spi_setDataMode(static_cast<h3_spi_mode_t>(m_SpiConfiguration.nMode)));

	DEBUG_EXIT
	return true;
}

void Serial::SendSpi(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	FUNC_PREFIX (spi_writenb(reinterpret_cast<const char *>(pData), nLength));

	DEBUG_EXIT
}
