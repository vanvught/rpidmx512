/**
 * @file ws28xxmulti8x.cpp
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
#include <string.h>
#include <cassert>

#include "ws28xxmulti.h"

#include "hal_gpio.h"
#include "hal_spi.h"

#include "debug.h"

#define SPI_CS1		GPIO_EXT_26

void WS28xxMulti::SetupHC595(uint8_t nT0H, uint8_t nT1H) {
	DEBUG_ENTRY

	nT0H = (nT0H << 1);
	nT1H = (nT1H << 1);

	DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", nT0H, nT1H);

	FUNC_PREFIX(gpio_fsel(SPI_CS1, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_set(SPI_CS1));

	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(1000000));

	FUNC_PREFIX(gpio_clr(SPI_CS1));
	FUNC_PREFIX(spi_write((nT1H << 8) | nT0H));
	FUNC_PREFIX(gpio_set(SPI_CS1));

	DEBUG_EXIT
}

void WS28xxMulti::SetupSPI() {
	DEBUG_ENTRY

	FUNC_PREFIX (spi_begin());
	FUNC_PREFIX(spi_chipSelect(SPI_CS0));
	FUNC_PREFIX(spi_set_speed_hz(6400000));

	DEBUG_EXIT
}

#define BIT_SET(a,b) 	((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) 	((a) &= ~(1<<(b)))

void WS28xxMulti::SetLED8x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPort < 8);
	assert(nLedIndex < m_nLedCount);

	uint32_t j = 0;
	const auto k = static_cast<uint32_t>(nLedIndex * SINGLE_RGB);

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		switch (m_tRGBMapping) {
		case RGB_MAPPING_RGB:
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_RBG:
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_GRB:
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_GBR:
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_BRG:
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			break;
		case RGB_MAPPING_BGR:
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			break;
		default:
			if (mask & nGreen) {
				BIT_SET(m_pBuffer8x[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[k + j], nPort);
			}
			if (mask & nRed) {
				BIT_SET(m_pBuffer8x[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
			}
			if (mask & nBlue) {
				BIT_SET(m_pBuffer8x[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
			}
			break;
		}

		j++;
	}
}

void WS28xxMulti::SetLED8x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPort < 8);
	assert(nLedIndex < m_nLedCount);
	assert(m_tWS28xxType == SK6812W);

	uint32_t j = 0;
	const auto k = static_cast<uint32_t>(nLedIndex * SINGLE_RGBW);

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		// GRBW
		if (mask & nGreen) {
			BIT_SET(m_pBuffer8x[k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer8x[k + j], nPort);
		}

		if (mask & nRed) {
			BIT_SET(m_pBuffer8x[8 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer8x[8 + k + j], nPort);
		}

		if (mask & nBlue) {
			BIT_SET(m_pBuffer8x[16 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer8x[16 + k + j], nPort);
		}

		if (mask & nWhite) {
			BIT_SET(m_pBuffer8x[24 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer8x[24 + k + j], nPort);
		}

		j++;
	}
}
