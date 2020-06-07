/**
 * @file ws28xxmulti4x.cpp
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
#include <string.h>
#include <cassert>

#include "ws28xxmulti.h"

#include "si5351a.h"
#include "mcp23017.h"
#include "mcp23x17.h"

#include "display.h"

#include "debug.h"

bool WS28xxMulti::SetupSI5351A(void) {
	DEBUG_ENTRY

	device_info_t clock_generator; // = { static_cast<spi_cs_t>(0), };
	memset(&clock_generator, 0, sizeof(device_info_t));

	if (!si5351a_start(&clock_generator)) {
		DEBUG_PUTS("si5351a not connected!");
		DEBUG_EXIT
		return false;
	} else {
		if (si5351a_clock_builder(&clock_generator)) {
			DEBUG_PUTS("si5351a is running");
			DEBUG_EXIT
			return true;
		} else {
			Display::Get()->TextStatus("E: SI5351A", Display7SegmentMessage::ERROR_SI5351A);
			DEBUG_PUTS("si5351a error");
			DEBUG_EXIT
			return false;
		}
	}

	__builtin_unreachable();
}

bool WS28xxMulti::IsMCP23017(void) {
	DEBUG_ENTRY

	device_info_t timing;// = { static_cast<spi_cs_t>(0), };
	memset(&timing, 0, sizeof(device_info_t));

	if (!mcp23017_start(&timing)) {
		puts("mcp23017 not connected!");
		DEBUG_EXIT
		return false;
	}

	return true;

	DEBUG_EXIT
}

bool WS28xxMulti::SetupMCP23017(uint8_t nT0H, uint8_t nT1H) {
	DEBUG_ENTRY

	device_info_t timing;// = { static_cast<spi_cs_t>(0), };
	memset(&timing, 0, sizeof(device_info_t));

	if (!mcp23017_start(&timing)) {
		puts("mcp23017 not connected!");
		DEBUG_EXIT
		return false;
	}

	mcp23017_reg_write(&timing, MCP23X17_IODIRA, 0x0000); // All output

	nT0H = (nT0H << 1);
	nT1H = (nT1H << 1);

	DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", nT0H, nT1H);

	mcp23017_reg_write(&timing, MCP23X17_GPIOA, (nT1H << 8) | nT0H);

	puts("mcp23017 running");
	DEBUG_EXIT
	return true;
}

#define BIT_SET(a,b) 	((a) |= (1U<<(b)))
#define BIT_CLEAR(a,b) 	((a) &= ~(1U<<(b)))

void WS28xxMulti::SetLED4x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPort < 4);
	assert(nLedIndex < m_nLedCount);

	uint32_t j = 0;
	uint32_t k = static_cast<uint32_t>(nLedIndex * SINGLE_RGB);

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		switch (m_tRGBMapping) {
		case RGB_MAPPING_RGB:
			if (mask & nRed) {
				BIT_SET(m_pBuffer4x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer4x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer4x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_RBG:
			if (mask & nRed) {
				BIT_SET(m_pBuffer4x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer4x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer4x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_GRB:
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
			break;
		case RGB_MAPPING_GBR:
			if (mask & nGreen) {
				BIT_SET(m_pBuffer4x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer4x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer4x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_BRG:
			if (mask & nBlue) {
				BIT_SET(m_pBuffer4x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer4x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer4x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_BGR:
			if (mask & nBlue) {
				BIT_SET(m_pBuffer4x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer4x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[16 + k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer4x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer4x[8 + k + j], nPort);
			}
			break;
		default:
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
			break;
		}

		j++;
	}
}

void WS28xxMulti::SetLED4x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPort < 4);
	assert(nLedIndex < m_nLedCount);
	assert(m_tWS28xxType == SK6812W);

	uint32_t j = 0;
	uint32_t k = static_cast<uint32_t>(nLedIndex * SINGLE_RGBW);

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
