/**
 * @file ltcdisplaymax72197segment.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYMAX72197SEGMENT_H_
#define LTCDISPLAYMAX72197SEGMENT_H_

#include <cstdint>

#include "ltcdisplaymax7219set.h"
#include "max72197segment.h"

class LtcDisplayMax72197Segment final: public LtcDisplayMax7219Set, public Max72197Segment {
public:
	LtcDisplayMax72197Segment();

	void Init(uint8_t nIntensity) override;
	void Show(const char *pTimecode) override;
	void ShowSysTime(const char *pSystemTime) override;
	void WriteChar(uint8_t nChar, uint8_t nPos) override;

	static LtcDisplayMax72197Segment *Get() {
		return s_pThis;
	}

private:
	static LtcDisplayMax72197Segment *s_pThis;
};

#endif /* LTCDISPLAYMAX72197SEGMENT_H_ */
