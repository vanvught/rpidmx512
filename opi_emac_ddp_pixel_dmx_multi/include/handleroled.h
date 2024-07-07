/**
 * @file handleroled.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef HANDLEROLED_H_
#define HANDLEROLED_H_

#include "jamstapl.h"

struct HandlerOled: public JamSTAPLDisplay  {
	HandlerOled() {
		s_pThis = this;
	}

	~HandlerOled() = default;

	// JamSTAPL
	void JamShowInfo(const char *pInfo) {
		Display::Get()->ClearLine(1);
		Display::Get()->Write(1, pInfo);
	}

	void JamShowStatus(const char *pStatus, int ExitCode) {
		Display::Get()->TextStatus(pStatus, ExitCode == 0 ? CONSOLE_GREEN : CONSOLE_RED);
	}

	static HandlerOled *Get(void) {
		return s_pThis;
	}

private:
	static HandlerOled *s_pThis;
};

HandlerOled *HandlerOled::s_pThis = 0;

#endif /* HANDLEROLED_H_ */
