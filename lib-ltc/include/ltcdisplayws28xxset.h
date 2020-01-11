/**
 * @file ltcdisplayws28xxset.h
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LTCDISPLAYWS28XXSET_H_
#define LTCDISPLAYWS28XXSET_H_

#include <stdint.h>

#include "ws28xx.h"

enum TLtcDisplayMessage {
	LTCDISPLAY_MAX_MESSAGE_SIZE = 8
};

struct TLtcDisplayRgbColours {
	uint8_t nRed;
	uint8_t nGreen;
	uint8_t nBlue;
};

class LtcDisplayWS28xxSet {
public:
	virtual ~LtcDisplayWS28xxSet(void) {}

	virtual void Init(TWS28XXType tLedType)=0;
	virtual void Print(void)=0;

	virtual void Show(const char *pTimecode, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons)=0;
	virtual void ShowSysTime(const char *pSystemTime, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons)=0;
	virtual void ShowMessage(const char *pMessage, struct TLtcDisplayRgbColours &tColours)=0;

	virtual void WriteChar(uint8_t nChar, uint8_t nPos, struct TLtcDisplayRgbColours &tColours)=0;
};

#endif /* LTCDISPLAYWS28XXSET_H_ */
