/**
 * @file displayedittimecode.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAYEDITTIMECODE_H_
#define DISPLAYEDITTIMECODE_H_

#include <stdint.h>

#include "ltc.h"

class DisplayEditTimeCode {
public:
	void HandleKey(int nKey, TLtcTimeCode& timecode, char m_aTimeCode[TC_CODE_MAX_LENGTH]);

private:
	void KeyUp(TLtcTimeCode &timecode);
	void KeyDown(TLtcTimeCode &timecode);
	void KeyLeft();
	void KeyRight();

private:
	uint8_t m_nFrames{0};
	enum State {
		IDLE, EDIT
	} m_State {IDLE};
	uint32_t m_nCursorPositionIndex{0};
	bool m_bCursorOn {false};
};

#endif /* DISPLAYEDITTIMECODE_H_ */
