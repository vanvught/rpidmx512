/**
 * @file ltcdisplayws28xx7segment.h
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
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

#ifndef LTCDISPLAYWS28XX7SEGMENT_H_
#define LTCDISPLAYWS28XX7SEGMENT_H_

#include <stdint.h>

#include "ws28xxdisplay7segment.h"

#include "ltcdisplayrgbset.h"

#include "rgbmapping.h"

class LtcDisplayWS28xx7Segment final: public LtcDisplayRgbSet {
public:
	LtcDisplayWS28xx7Segment();

	void Init(TWS28XXType tLedType = WS2812B, TRGBMapping tRGBMapping = RGB_MAPPING_UNDEFINED) override;

	void Show(const char *pTimecode, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) override;
	void ShowSysTime(const char *pSystemTime, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) override;
	void ShowMessage(const char *pMessage , struct TLtcDisplayRgbColours &tColours) override;

	void WriteChar(uint8_t nChar, uint8_t nPos, struct TLtcDisplayRgbColours &tColours) override;

	void Print() override;

private:
	WS28xxDisplay7Segment *m_pWS28xxDisplay7Segment;
};

#endif /* INCLUDE_LTCDISPLAYWS28XX7SEGMENT_H_ */
