/**
 * @file systimereader.h
 *
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

#ifndef H3_SYSTIMEREADER_H_
#define H3_SYSTIMEREADER_H_

#include <stdint.h>

#include "ltc.h"

class SystimeReader {
public:
	SystimeReader (struct TLtcDisabledOutputs *pLtcDisabledOutputs, enum TTimecodeTypes tTimecodeType);
	~SystimeReader(void);

	void Start(void);
	void Run(void);

private:
	struct TLtcDisabledOutputs *m_ptLtcDisabledOutputs;
	enum TTimecodeTypes m_tTimecodeType;
	uint8_t m_nFps;
	uint32_t m_nTimer0Interval;
};


#endif /* H3_SYSTIMEREADER_H_ */
