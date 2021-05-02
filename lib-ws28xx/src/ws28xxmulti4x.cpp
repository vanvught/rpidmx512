/**
 * @file ws28xxmulti4x.cpp
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

#include <cstdio>
#include <cstring>
#include <cassert>

#include "ws28xxmulti.h"

#include "si5351a.h"
#include "mcp23x17.h"
#include "hal_i2c.h"

#include "debug.h"

using namespace pixel;

bool WS28xxMulti::SetupSI5351A(	) {
	DEBUG_ENTRY

	SI5351A si5351a;

	if (!si5351a.IsConnected()) {
		DEBUG_PUTS("SI5351A is not connected!");
		DEBUG_EXIT
		return false;
	}

	si5351a.ClockBuilder();

	DEBUG_PUTS("SI5351A is running");
	DEBUG_EXIT
	return true;
}

bool WS28xxMulti::IsMCP23017() {
	return HAL_I2C::IsConnected(mcp23x17::i2c::address, hal::i2c::NORMAL_SPEED);
}

bool WS28xxMulti::SetupMCP23017(uint8_t nT0H, uint8_t nT1H) {
	DEBUG_ENTRY

	HAL_I2C MCP23017(mcp23x17::i2c::address);

	if (!MCP23017.IsConnected()) {
		puts("MCP23017 not connected!");
		DEBUG_EXIT
		return false;
	}

	MCP23017.WriteRegister(mcp23x17::reg::IODIRA, static_cast<uint16_t>(0x0000)); // All output

	nT0H = (nT0H << 1);
	nT1H = (nT1H << 1);

	DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", nT0H, nT1H);

	MCP23017.WriteRegister(mcp23x17::reg::GPIOA, static_cast<uint16_t>(nT1H | (nT0H << 8)));

	puts("MCP23017 is configured");
	DEBUG_EXIT
	return true;
}

#define BIT_SET(a,b) 	((a) |= (1U<<(b)))
#define BIT_CLEAR(a,b) 	((a) &= ~(1U<<(b)))

void  WS28xxMulti::SetColour4x(uint32_t nPort, uint32_t nLedIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
	uint32_t j = 0;
	const auto k = nLedIndex * pixel::single::RGB;

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		if (mask & nColour1) {
			BIT_SET(m_pBuffer4x[k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[k + j], nPort);
		}
		if (mask & nColour2) {
			BIT_SET(m_pBuffer4x[8 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
		}
		if (mask & nColour3) {
			BIT_SET(m_pBuffer4x[16 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
		}

		j++;
	}
}

void WS28xxMulti::SetPixel4x(uint32_t nPort, uint32_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPort < 4);
	assert(nLedIndex < m_nCount);

	switch (m_Map) {
	case Map::RGB:
		SetColour4x(nPort, nLedIndex, nRed, nGreen, nBlue);
		break;
	case Map::RBG:
		SetColour4x(nPort, nLedIndex, nRed, nBlue, nGreen);
		break;
	case Map::GRB:
		SetColour4x(nPort, nLedIndex, nGreen, nRed, nBlue);
		break;
	case Map::GBR:
		SetColour4x(nPort, nLedIndex, nGreen, nBlue, nRed);
		break;
	case Map::BRG:
		SetColour4x(nPort, nLedIndex, nBlue, nRed, nGreen);
		break;
	case Map::BGR:
		SetColour4x(nPort, nLedIndex, nBlue, nGreen, nRed);
		break;
	default:
		SetColour4x(nPort, nLedIndex, nGreen, nRed, nBlue);
		break;
	}
}

void WS28xxMulti::SetPixel4x(uint32_t nPort, uint32_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPort < 4);
	assert(nLedIndex < m_nCount);
	assert(m_Type == Type::SK6812W);

	uint32_t j = 0;
	auto k = nLedIndex * pixel::single::RGBW;

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		// GRBW
		if (mask & nGreen) {
			BIT_SET(m_pBuffer4x[k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[k + j], nPort);
		}

		if (mask & nRed) {
			BIT_SET(m_pBuffer4x[8 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
		}

		if (mask & nBlue) {
			BIT_SET(m_pBuffer4x[16 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
		}

		if (mask & nWhite) {
			BIT_SET(m_pBuffer4x[24 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer4x[24 + k + j], nPort);
		}

		j++;
	}
}
