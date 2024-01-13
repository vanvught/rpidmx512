/**
 * @file mcp23s17.h
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

#ifndef MCP23S17_H_
#define MCP23S17_H_

#include <cstdint>

#include "hal_spi.h"

#include "mcp23x17.h"

namespace gpio {

class MCP23S17 : HAL_SPI {
public:
	MCP23S17(uint8_t nChipSelect = 0, uint32_t nSpeedHz = mcp23x17::SPI_SPEED_DEFAULT_HZ, uint8_t nAddress = 0):
		HAL_SPI(nChipSelect, nSpeedHz == 0 ? mcp23x17::SPI_SPEED_DEFAULT_HZ :
						(mcp23x17::SPI_SPEED_DEFAULT_HZ <= mcp23x17::SPI_SPEED_MAX_HZ ? nSpeedHz : mcp23x17::SPI_SPEED_MAX_HZ)),
						m_nAddress(nAddress) {
		MCP23S17::WriteRegister(mcp23x17::REG_IOCON, mcp23x17::IOCON_HAEN);
	}

	void WriteRegister(uint8_t nRegister, uint8_t nValue) {
		char spiData[3];

		spiData[0] = static_cast<char>(mcp23x17::SPI_CMD_WRITE) | static_cast<char>(m_nAddress << 1);
		spiData[1] = static_cast<char>(nRegister);
		spiData[2] = static_cast<char>(nValue);

		HAL_SPI::Write(spiData, 3);
	}

	void WriteRegister(uint8_t nRegister, uint16_t nValue) {
		char spiData[4];

		spiData[0] = static_cast<char>(mcp23x17::SPI_CMD_WRITE) | static_cast<char>(m_nAddress << 1);
		spiData[1] = static_cast<char>(nRegister);
		spiData[2] = static_cast<char>(nValue);
		spiData[3] = static_cast<char>(nValue >> 8);

		HAL_SPI::Write(spiData, 4);
	}

private:
	uint8_t m_nAddress = 0;
};

}  // namespace gpio

#endif /* MCP23S17_H_ */
