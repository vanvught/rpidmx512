/**
 * @file ws28xxmulti.cpp
 *
 */
/**
 * Stub for testing purpose only
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

#include "debug.h"

enum {
	LEDCOUNT_RGB_MAX = (4 * 170), LEDCOUNT_RGBW_MAX = (4 * 128)
};

enum {
	SINGLE_RGB = 24, SINGLE_RGBW = 32
};

#define PULSE 	(6)

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
	DEBUG_ENTRY

	delete [] m_pBlackoutBuffer;
	m_pBlackoutBuffer = 0;

	delete [] m_pBuffer;
	m_pBuffer = 0;

	DEBUG_EXIT
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
	//TODO Implement set RGBW
}

void WS28xxMulti::Update(void) {
	Generate800kHz(m_pBuffer);
}

void WS28xxMulti::Blackout(void) {
	Generate800kHz(m_pBlackoutBuffer);
}

void WS28xxMulti::Generate800kHz(const uint32_t* pBuffer) {
	for (uint32_t k = 0; k < m_nBufSize; k = k + SINGLE_RGB) {
		printf("%.2d ", k / SINGLE_RGB);

		for (uint32_t j = 0; j < 4; j++) {

			for (uint32_t i = 0; i < 24; i++) {
				bool b = (pBuffer[k + i] & (1 << j)) == 0;
				printf("%c", '0' + (b ? 0 : 1));

				if ((i != 0) && ((i + 1) % 8 == 0)) {
					printf("|");
				}
			}
			printf(" ");
		}
		printf("\n");
	}

	printf("\n");
}
