/**
 * @file displayeditfps.cpp
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

#include <stdint.h>

#include "displayeditfps.h"

#include "input.h"

#include "ltc.h"

#include "display.h"
#include "ltc7segment.h"

void DisplayEditFps::HandleKey(int nKey, uint8_t &nType) {

	if (m_State == IDLE) {
		if (nKey == INPUT_KEY_ENTER) {
			m_State = EDIT;
			m_bCursorOn = true;
		}
	} else {
		switch (nKey) {
		case INPUT_KEY_ESC:
			m_State = IDLE;
			m_bCursorOn = false;
			break;
		case INPUT_KEY_DOWN:
		case INPUT_KEY_LEFT:
			KeyLeft(nType);
			break;
		case INPUT_KEY_UP:
		case INPUT_KEY_RIGHT:
			KeyRight(nType);
			break;
		default:
			break;
		}
	}

	Display::Get()->TextLine(2, Ltc::GetType(static_cast<ltc::type>(nType)), TC_TYPE_MAX_LENGTH);
	Ltc7segment::Get()->Show(static_cast<ltc::type>(nType));

	if (m_bCursorOn) {
		Display::Get()->SetCursor(display::cursor::ON);
		Display::Get()->SetCursorPos(0, 1);
	} else {
		Display::Get()->SetCursor(display::cursor::OFF);
	}
}

void DisplayEditFps::KeyLeft(uint8_t &nType) {
	if (nType > 0) {
		nType--;
		return;
	}
	nType = 3;
}

void DisplayEditFps::KeyRight(uint8_t &nType) {
	if (nType < 3) {
		nType++;
		return;
	}
	nType = 0;
}
