/**
 * @file ltcdisplayrgbset.h
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

#ifndef LTCDISPLAYRGBSET_H_
#define LTCDISPLAYRGBSET_H_

#include <stdint.h>

#include "ltc.h"
#include "ws28xx.h"

#include "rgbmapping.h"

namespace ltcdisplayrgb {
struct Colours {
	uint8_t nRed;
	uint8_t nGreen;
	uint8_t nBlue;
};
static constexpr auto MAX_MESSAGE_SIZE = 8;
}  // namespace ltcdisplayrgb

class LtcDisplayRgbSet {
public:
	virtual ~LtcDisplayRgbSet() {}

	virtual void Init(TWS28XXType tLedType, TRGBMapping tRGBMapping);
	virtual void Init();

	virtual void Show(const char *pTimecode, struct ltcdisplayrgb::Colours &tColours, struct ltcdisplayrgb::Colours &tColoursColons)=0;
	virtual void ShowSysTime(const char *pSystemTime, struct ltcdisplayrgb::Colours &tColours, struct ltcdisplayrgb::Colours &tColoursColons)=0;
	virtual void ShowMessage(const char *pMessage, struct ltcdisplayrgb::Colours &tColours)=0;

	virtual void WriteChar(uint8_t nChar, uint8_t nPos, struct ltcdisplayrgb::Colours &tColours)=0;

	virtual void ShowFPS(ltc::type tTimeCodeType, struct ltcdisplayrgb::Colours &tColours);
	virtual void ShowSource(ltc::source tSource, struct ltcdisplayrgb::Colours &tColours);
	virtual void ShowInfo(const char *pInfo, uint32_t nLength, struct ltcdisplayrgb::Colours &tColours);

	virtual void Print()=0;
};

#endif /* LTCDISPLAYRGBSET_H_ */
