/**
 * @file displayhandler.h
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAYHANDLER_H_
#define DISPLAYHANDLER_H_

#include <cstdint>

#include "ledblink.h"
#include "display.h"

struct DisplayHandler final: public LedBlinkDisplay {
	DisplayHandler() : m_bHaveDisplay(Display::Get() != nullptr) {
		if (m_bHaveDisplay) {
			m_bHaveDisplay = Display::Get()->isDetected();
		}
	}

	~DisplayHandler() {
	}

	void Print(uint32_t nMode) {
		if (m_bHaveDisplay) {
			char c;
			switch (static_cast<ledblink::Mode>(nMode)) {
			case ledblink::Mode::OFF_OFF:
				c = 'O';
				break;
			case ledblink::Mode::OFF_ON:
				c = 'O';
				break;
			case ledblink::Mode::NORMAL:
				c = 'N';
				break;
			case ledblink::Mode::DATA:
				c = 'D';
				break;
			case ledblink::Mode::FAST:
				c = 'F';
				break;
			default:
				c = 'U';
				break;
			}

			Display::Get()->SetCursorPos(static_cast<uint8_t>(Display::Get()->GetColumns() - 1U), static_cast<uint8_t>(Display::Get()->GetRows() - 1U));
			Display::Get()->PutChar(c);
		}
	}

private:
	bool m_bHaveDisplay;
};

#endif /* DISPLAYHANDLER_H_ */
