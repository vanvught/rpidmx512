/**
 * @file serialspi.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "../src/serial/serial.h"

#include "h3_spi.h"	// TODO Replace with hal_spi.h ?

#include "debug.h"

using namespace serial;

void Serial::SetSpiSpeedHz(uint32_t nSpeedHz) {
	DEBUG_PRINTF("nSpeedHz=%d", nSpeedHz);

	if (nSpeedHz == 0) {
		return;

	}

	m_SpiConfiguration.nSpeed = nSpeedHz;
}

void Serial::SetSpiMode(spi::mode tMode) {
	DEBUG_PRINTF("tMode=%d", tMode);

	if (static_cast<int>(tMode) > 3) {
		return;
	}

	m_SpiConfiguration.nMode = static_cast<uint8_t>(tMode);
}

bool Serial::InitSpi(void) {
	DEBUG_ENTRY

	h3_spi_begin();
	h3_spi_set_speed_hz(m_SpiConfiguration.nSpeed);
	h3_spi_chipSelect(H3_SPI_CS0);
	h3_spi_setDataMode(static_cast<h3_spi_mode_t>(m_SpiConfiguration.nMode));

	DEBUG_EXIT
	return true;
}

void Serial::SendSpi(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	h3_spi_writenb(reinterpret_cast<const char *>(pData), nLength);

	DEBUG_EXIT
}
