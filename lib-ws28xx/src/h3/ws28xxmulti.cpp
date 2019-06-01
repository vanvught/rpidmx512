/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "ws28xxmulti.h"

#include "h3_gpio.h"

#include "si5351a.h"
#include "mcp23017.h"
#include "mcp23x17.h"

#include "display.h"

#include "debug.h"

enum {
	LEDCOUNT_RGB_MAX = (4 * 170), LEDCOUNT_RGBW_MAX = (4 * 128)
};

enum {
	SINGLE_RGB = 24, SINGLE_RGBW = 32
};

#define PULSE 	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 6)	// Pin 7
#define ENABLE	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 18)	// Pin 18

#define OUT0	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 0)	// Pin 13
#define OUT1	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 1)	// Pin 11
#define OUT2	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 2)	// Pin 22
#define OUT3	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 3)	// Pin 15

#define DATA_MASK	((1 << PULSE) | (1 << ENABLE) | (1 << OUT3) | (1 << OUT2) | (1 << OUT1) | (1 << OUT0))

#define BIT_SET(a,b) 	((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) 	((a) &= ~(1<<(b)))

static TWS28xxMultiType s_NotSupported[] = {WS28XXMULTI_WS2801_NOT_SUPPORTED, WS28XXMULTI_APA102_NOT_SUPPORTED};

WS28xxMulti::WS28xxMulti(TWS28xxMultiType tWS28xxMultiType, uint16_t nLedCount, uint8_t nActiveOutputs, uint8_t nT0H, uint8_t nT1H, bool bUseSI5351A):
	m_tWS28xxMultiType(tWS28xxMultiType),
	m_nLedCount(nLedCount),
	m_nActiveOutputs(nActiveOutputs),
	m_nLowCode(CalculateBits(nT0H)),
	m_nHighCode(CalculateBits(nT1H)),
	m_nBufSize(0),
	m_pBuffer(0),
	m_pBlackoutBuffer(0)
{
	DEBUG_ENTRY

	assert(tWS28xxMultiType <= WS28XXMULTI_UCS1903);
	assert(nLedCount > 0);
	assert(nActiveOutputs <= WS28XXMULTI_ACTIVE_PORTS_MAX);

	for (uint32_t i = 0; i < sizeof(s_NotSupported) / sizeof(s_NotSupported[0]) ; i++) {
		if (tWS28xxMultiType == s_NotSupported[i]) {
			m_tWS28xxMultiType = WS28XXMULTI_WS2812B;
			break;
		}
	}

	if (m_tWS28xxMultiType == WS28XXMULTI_SK6812W) {
		m_nLedCount =  nLedCount <= LEDCOUNT_RGBW_MAX ? nLedCount : LEDCOUNT_RGBW_MAX;
		m_nBufSize = nLedCount * SINGLE_RGBW;
	} else {
		m_nLedCount =  nLedCount <= LEDCOUNT_RGB_MAX ? nLedCount : LEDCOUNT_RGB_MAX;
		m_nBufSize = nLedCount * SINGLE_RGB;
	}

	DEBUG_PRINTF("type=%d, count=%d, active=%d, bufsize=%d", m_tWS28xxMultiType, m_nLedCount, m_nActiveOutputs, m_nBufSize);

	h3_gpio_fsel(OUT0, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT0);
	h3_gpio_fsel(OUT1, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT1);
	h3_gpio_fsel(OUT2, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT2);
	h3_gpio_fsel(OUT3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT3);

	h3_gpio_fsel(PULSE, GPIO_FSEL_OUTPUT);
	h3_gpio_set(PULSE);

	h3_gpio_fsel(ENABLE, GPIO_FSEL_OUTPUT);
	h3_gpio_set(ENABLE);

	if (bUseSI5351A) {
		SetupSI5351A();
	}

	m_nLowCode = 0xC0;
	m_nHighCode = (m_tWS28xxMultiType == WS28XXMULTI_WS2812B ? 0xF8 : (((m_tWS28xxMultiType == WS28XXMULTI_UCS1903) || (m_tWS28xxMultiType == WS28XXMULTI_UCS2903)) ? 0xFC : 0xF0));

	SetupMCP23017(ReverseBits(m_nLowCode), ReverseBits(m_nHighCode));

	m_pBuffer = new uint32_t[m_nBufSize];
	assert(m_pBuffer != 0);

	m_pBlackoutBuffer = new uint32_t[m_nBufSize];
	assert(m_pBlackoutBuffer != 0);

	for (uint32_t i = 0; i < m_nBufSize; i++) {
		uint32_t d = (i & 0x1) ? (1 << PULSE) : 0;
		m_pBuffer[i] = d;
		m_pBlackoutBuffer[i] = d;
	}

	DEBUG_EXIT
}

WS28xxMulti::~WS28xxMulti(void) {
	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;
}

void WS28xxMulti::SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPort < WS28XXMULTI_ACTIVE_PORTS_MAX);
	assert(nLedIndex < m_nLedCount);

	uint32_t j = 0;
	uint32_t k = nLedIndex * SINGLE_RGB;

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		if (m_tWS28xxMultiType == WS28XXMULTI_WS2811) {
			// RGB
			if (mask & nRed) {
				BIT_SET(m_pBuffer[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[k + j], nPort);
			}

			if (mask & nGreen) {
				BIT_SET(m_pBuffer[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[8 + k + j], nPort);
			}

			if (mask & nBlue) {
				BIT_SET(m_pBuffer[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[16 + k + j], nPort);
			}
		} else if  (m_tWS28xxMultiType == WS28XXMULTI_UCS1903) {
			// BRG
			if (mask & nBlue) {
				BIT_SET(m_pBuffer[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[k + j], nPort);
			}

			if (mask & nRed) {
				BIT_SET(m_pBuffer[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[8 + k + j], nPort);
			}

			if (mask & nGreen) {
				BIT_SET(m_pBuffer[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[16 + k + j], nPort);
			}
		} else {
			// GRB
			if (mask & nGreen) {
				BIT_SET(m_pBuffer[k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[k + j], nPort);
			}

			if (mask & nRed) {
				BIT_SET(m_pBuffer[8 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[8 + k + j], nPort);
			}

			if (mask & nBlue) {
				BIT_SET(m_pBuffer[16 + k + j], nPort);
			} else {
				BIT_CLEAR(m_pBuffer[16 + k + j], nPort);
			}
		}

		j++;
	}
}

void WS28xxMulti::SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPort < WS28XXMULTI_ACTIVE_PORTS_MAX);
	assert(nLedIndex < m_nLedCount);
	assert(m_tWS28xxMultiType == WS28XXMULTI_SK6812W);

	uint32_t j = 0;
	uint32_t k = nLedIndex * SINGLE_RGBW;

	for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		// GRB
		if (mask & nGreen) {
			BIT_SET(m_pBuffer[k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer[k + j], nPort);
		}

		if (mask & nRed) {
			BIT_SET(m_pBuffer[8 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer[8 + k + j], nPort);
		}

		if (mask & nBlue) {
			BIT_SET(m_pBuffer[16 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer[16 + k + j], nPort);
		}

		if (mask & nWhite) {
			BIT_SET(m_pBuffer[24 + k + j], nPort);
		} else {
			BIT_CLEAR(m_pBuffer[24 + k + j], nPort);
		}

		j++;
	}
}

void WS28xxMulti::Update(void) {
	Generate800kHz(m_pBuffer);
}

void WS28xxMulti::Blackout(void) {
	DEBUG_ENTRY

	Generate800kHz(m_pBlackoutBuffer);

	DEBUG_EXIT
}

void WS28xxMulti::Generate800kHz(const uint32_t* pBuffer) {
	uint32_t i = 0;
	const uint32_t d = (125 * 24) / 100;
	uint32_t dat;

	do {
		uint64_t cval;
		asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));

		dat = H3_PIO_PORTA->DAT;
		dat &= (~(DATA_MASK));
		dat |= pBuffer[i];
		H3_PIO_PORTA->DAT = dat;

		uint32_t t1 = (uint32_t) (cval & 0xFFFFFFFF);
		const uint32_t t2 = t1 + d;
		i++;

		__builtin_prefetch(&pBuffer[i]);

		do {
			asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
			t1 = (uint32_t) (cval & 0xFFFFFFFF);
		} while (t1 < t2);

	} while (i < m_nBufSize);

	dat |= (1 << ENABLE);
	H3_PIO_PORTA->DAT = dat;
}

uint8_t WS28xxMulti::ReverseBits(uint8_t nBits) {
	const uint32_t input = (uint32_t) nBits;
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return (uint8_t) (output >> 24);
}

bool WS28xxMulti::SetupSI5351A(void) {
	device_info_t clock_generator = { (spi_cs_t) 0, };

	if (!si5351a_start(&clock_generator)) {
		Display::Get()->TextStatus("E: SI5351A", DISPLAY_7SEGMENT_MSG_ERROR_SI5351A);
		DEBUG_PUTS("si5351a not connected!");
		return false;
	} else {
		if (si5351a_clock_builder(&clock_generator)) {
			DEBUG_PUTS("si5351a is running");
			return true;
		} else {
			DEBUG_PUTS("si5351a error");
			return false;
		}
	}

	__builtin_unreachable();
}

bool WS28xxMulti::SetupMCP23017(uint8_t nT0H, uint8_t nT1H) {
	device_info_t timing = { (spi_cs_t) 0, };

	if (!mcp23017_start(&timing)) {
		Display::Get()->TextStatus("E: MCP23017", DISPLAY_7SEGMENT_MSG_ERROR_MCP23S017);
		DEBUG_PUTS("mcp23017 not connected!");
		return false;
	}

	mcp23017_reg_write(&timing, MCP23X17_IODIRA, 0x0000); // All output

	nT0H = (nT0H << 1);
	nT1H = (nT1H << 1);

	DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", nT0H, nT1H);

	mcp23017_reg_write(&timing, MCP23X17_GPIOA, (uint16_t) (nT1H << 8) | nT0H);

	DEBUG_PUTS("mcp23017 running");

	return true;
}

uint8_t WS28xxMulti::CalculateBits(uint8_t nNanoSeconds) {
	return 0;
}
