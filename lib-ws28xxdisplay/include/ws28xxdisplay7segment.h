/**
 * @file ws28xxdisplay7segment.h
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined(USE_SPI_DMA)
# include "h3/ws28xxdma.h"
#endif
#include "ws28xx.h"

#include "rgbmapping.h"

struct WS28xxDisplay7SegmentConfig {
	static constexpr uint32_t NUM_OF_DIGITS = 8;
	static constexpr uint32_t NUM_OF_COLONS = 3;

	static constexpr uint32_t SEGMENTS_PER_DIGIT = 7;	///< number of LEDs that make up one digit
	static constexpr uint32_t LEDS_PER_SEGMENT = 1;		///< number of LEDs that make up one segment
	static constexpr uint32_t LEDS_PER_COLON = 2;		///< number of LEDs that make up one colon

	static constexpr uint32_t LED_COUNT = ((WS28xxDisplay7SegmentConfig::LEDS_PER_SEGMENT * WS28xxDisplay7SegmentConfig::SEGMENTS_PER_DIGIT * WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS)
			+ (WS28xxDisplay7SegmentConfig::LEDS_PER_COLON * WS28xxDisplay7SegmentConfig::NUM_OF_COLONS));
};

class WS28xxDisplay7Segment {
public:
	WS28xxDisplay7Segment();
	~WS28xxDisplay7Segment();

	void Init(TWS28XXType tLedType = WS2812B, TRGBMapping tRGBMapping = RGB_MAPPING_UNDEFINED);

	void WriteChar(char nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void WriteColon(char nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void WriteAll(const char *pChars, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void SetColonsOff();

	void Show();

private:
	void RenderSegment(bool bOnOff, uint32_t nCurrentDigitBase, uint32_t nCurrentSegment, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

private:
#if defined(USE_SPI_DMA)
	WS28xxDMA *m_pWS28xx{nullptr};
#else
	WS28xx *m_pWS28xx{nullptr};
#endif
};

#endif /* WS28XXDISPLAY7SEGMENT_H_ */
