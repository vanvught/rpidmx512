/**
 * @file ws28xxdisplay7segment.cpp
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "ws28xxdisplay7segment.h"
#include "ws28xxdisplay7segment_font.h"

#if defined(USE_SPI_DMA)
 #include "h3/ws28xxdma.h"
#else
 #include "ws28xx.h"
#endif

#include "debug.h"

/*
 *  A 8 x 7 Segment (with 3 colons) TC Display constructed of WS82xx LEDS,
 *
 *   AAA
 *	F	  B
 *	F	  B
 *	F	  B
 *	 GGG     x 8
 * 	E	  C
 * 	E	  C
 * 	E	  C
 *   DDD
 *
 *  Then the colons x 3 at the end.
 *
*/

WS28xxDisplay7Segment::WS28xxDisplay7Segment(void):
	m_pWS28xx(0)
{
	DEBUG2_ENTRY

	DEBUG2_EXIT
}

WS28xxDisplay7Segment::~WS28xxDisplay7Segment(void) {
	DEBUG2_ENTRY

	if (m_pWS28xx != 0) {
		delete m_pWS28xx;
		m_pWS28xx = 0;
	}

	DEBUG2_EXIT
}

void WS28xxDisplay7Segment::Init(TWS28XXType tLedType) {
	DEBUG2_ENTRY

	assert(m_pWS28xx == 0);
#if defined(USE_SPI_DMA)
	m_pWS28xx = new WS28xxDMA(tLedType, WS28XX_LED_COUNT);
#else
	m_pWS28xx = new WS28xx(tLedType, WS28XX_LED_COUNT);
#endif
	assert(m_pWS28xx != 0);

	m_pWS28xx->Initialize();

	DEBUG2_EXIT
}

void WS28xxDisplay7Segment::WriteChar(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nChar > sizeof(Seg7Array)) {
		return;
	}

	const uint32_t nCurrentDigitBase = nPos * SEGMENTS_PER_DIGIT;

	uint8_t chr;

	if (nChar & (1 << 7)) {	// use custom bitmap
		chr = nChar;
	} else {				// use displayws28xx_font
		chr = Seg7Array[nChar];
	}

	RenderSegment(chr & (1 << 6), nCurrentDigitBase, 0, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 5), nCurrentDigitBase, 1, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 4), nCurrentDigitBase, 2, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 3), nCurrentDigitBase, 3, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 2), nCurrentDigitBase, 4, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 1), nCurrentDigitBase, 5, nRed, nGreen, nBlue);
	RenderSegment(chr & (1 << 0), nCurrentDigitBase, 6, nRed, nGreen, nBlue);
}

void WS28xxDisplay7Segment::WriteColon(uint8_t nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	const uint32_t nCurrentDigitBase = (WS28XX_NUM_OF_DIGITS * SEGMENTS_PER_DIGIT) + (nPos * LEDS_PER_COLON);
	const bool OnOff = (nChar == ':' || nChar == '.' || nChar == ';') ? 1 : 0;

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t nIndex = nCurrentDigitBase; nIndex < (nCurrentDigitBase + LEDS_PER_COLON); nIndex++) {
		if (OnOff) {
			m_pWS28xx->SetLED(nIndex, nRed, nGreen, nBlue);
		} else {
			m_pWS28xx->SetLED(nIndex, 0x00, 0x00, 0x00);
		}
	}
}

void WS28xxDisplay7Segment::SetColonsOff(void) {
	for (uint32_t nCount = 0; nCount < WS28XX_NUM_OF_COLONS; nCount++) {
		WriteColon(' ', nCount, 0x00, 0x00, 0x00);
	}
}

void WS28xxDisplay7Segment::WriteAll(const uint8_t *pChars, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	for (uint32_t nPos = 0; nPos < WS28XX_NUM_OF_DIGITS; nPos++) {
		WriteChar(pChars[nPos], nPos, nRed, nGreen, nBlue);
	}
}

void WS28xxDisplay7Segment::RenderSegment(bool bOnOff, uint32_t nCurrentDigitBase, uint32_t nCurrentSegment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	const uint32_t nCurrentSegmentBase = nCurrentDigitBase + (nCurrentSegment * LEDS_PER_SEGMENT);

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t nIndex = nCurrentSegmentBase; nIndex < (nCurrentSegmentBase + LEDS_PER_SEGMENT); nIndex++) {
		if (bOnOff) {
			m_pWS28xx->SetLED(nIndex, nRed, nGreen, nBlue); // on
		} else {
			m_pWS28xx->SetLED(nIndex, 0, 0, 0); // off
		}
	}
}

void WS28xxDisplay7Segment::Show(void) {
//	if (m_bUpdateNeeded) {
//		m_bUpdateNeeded = false;
		m_pWS28xx->Update();
//	}
}

