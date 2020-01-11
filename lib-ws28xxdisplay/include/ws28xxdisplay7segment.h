/**
 * @file ws28xxdisplay7segment.h
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

#ifndef WS28XXDISPLAY7SEGMENT_H_
#define WS28XXDISPLAY7SEGMENT_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(USE_SPI_DMA)
 #include "h3/ws28xxdma.h"
#endif
#include "ws28xx.h"

#define WS28XX_NUM_OF_DIGITS	8
#define WS28XX_NUM_OF_COLONS	3

#define SEGMENTS_PER_DIGIT 		7		///< number of LEDs that make up one digit
#define LEDS_PER_SEGMENT 		1		///< number of LEDs that make up one segment
#define LEDS_PER_COLON 			2		///< number of LEDs that make up one colon

#define WS28XX_LED_COUNT 		((LEDS_PER_SEGMENT * SEGMENTS_PER_DIGIT * WS28XX_NUM_OF_DIGITS) + (LEDS_PER_COLON * WS28XX_NUM_OF_COLONS))

class WS28xxDisplay7Segment {
public:
	WS28xxDisplay7Segment(void);
	~WS28xxDisplay7Segment(void);

	void Init(TWS28XXType tLedType = WS2812B);

	void WriteChar(uint8_t nChar, uint8_t nPos, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);
	void WriteColon(uint8_t nChar, uint8_t nPos, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);

	void WriteAll(const uint8_t *pChars, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);

	void SetColonsOff(void);

	void Show(void);

private:
	void RenderSegment(bool bOnOff, uint32_t nCurrentDigitBase, uint32_t nCurrentSegment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

private:
#if defined(USE_SPI_DMA)
	WS28xxDMA *m_pWS28xx;
#else
	WS28xx *m_pWS28xx;
#endif
};

#endif /* WS28XXDISPLAY7SEGMENT_H_ */
