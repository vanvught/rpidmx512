/**
 * @file ltcdisplayws28xxmatrix.h
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LTCDISPLAYWS28XXMATRIX_H_
#define LTCDISPLAYWS28XXMATRIX_H_

#include <stdint.h>

#include "ltcdisplayrgbset.h"

#include "ws28xxdisplaymatrix.h"

#include "rgbmapping.h"

class LtcDisplayWS28xxMatrix final: public LtcDisplayRgbSet {
public:
	LtcDisplayWS28xxMatrix();
	~LtcDisplayWS28xxMatrix();

	void Init(TWS28XXType tLedType = WS2812B, TRGBMapping tRGBMapping = RGB_MAPPING_UNDEFINED) override;
	void Print() override;

	void Show(const char *pTimecode, struct ltcdisplayrgb::Colours &tColours, struct ltcdisplayrgb::Colours &tColoursColons) override;
	void ShowSysTime(const char *pSystemTime, struct ltcdisplayrgb::Colours &tColours, struct ltcdisplayrgb::Colours &tColoursColons) override;
	void ShowMessage(const char *pMessage , struct ltcdisplayrgb::Colours &tColours) override;

	void WriteChar(uint8_t nChar, uint8_t nPos, struct ltcdisplayrgb::Colours &tColours) override;

private:
	WS28xxDisplayMatrix *m_pWS28xxDisplayMatrix;
};

#endif /* LTCDISPLAYWS28XXMATRIX_H_ */
