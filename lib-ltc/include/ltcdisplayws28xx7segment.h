/**
 * @file ltcdisplayws28xx7segment.h
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "ws28xxdisplay7segment.h"

#include "ltcdisplayrgbset.h"

class LtcDisplayWS28xx7Segment final: public LtcDisplayRgbSet {
public:
	LtcDisplayWS28xx7Segment(pixel::Type type, pixel::Map map);

	void Show(const char *pTimecode, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) override;
	void ShowSysTime(const char *pSystemTime, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) override;
	void ShowMessage(const char *pMessage , struct ltcdisplayrgb::Colours& colours) override;
	void WriteChar(uint8_t nChar, uint8_t nPos, struct ltcdisplayrgb::Colours& colours) override;
	void Print() override;

private:
	WS28xxDisplay7Segment *m_pWS28xxDisplay7Segment;
};

#endif /* INCLUDE_LTCDISPLAYWS28XX7SEGMENT_H_ */
