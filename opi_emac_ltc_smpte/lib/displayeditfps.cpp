/**
 * @file displayeditfps.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "displayeditfps.h"

#include "input.h"

#include "ltc.h"

#include "display.h"

#include "debug.h"

static void key_left(uint8_t &nType) {
	if (nType > 0) {
		nType--;
		return;
	}
	nType = 3;
}

static void key_right(uint8_t &nType) {
	if (nType < 3) {
		nType++;
		return;
	}
	nType = 0;
}

void DisplayEditFps::HandleKey(int nKey, uint8_t &nType) {
	DEBUG_PRINTF("%d %d", m_State, nKey);

	if (m_State == IDLE) {
		if (nKey == input::KEY_ENTER) {
			m_State = EDIT;
			m_bCursorOn = true;
		}
	} else {
		switch (nKey) {
		case input::KEY_ESC:
			m_State = IDLE;
			m_bCursorOn = false;
			break;
		case input::KEY_DOWN:
		case input::KEY_LEFT:
			key_left(nType);
			break;
		case input::KEY_UP:
		case input::KEY_RIGHT:
			key_right(nType);
			break;
		default:
			break;
		}
	}

	Display::Get()->TextLine(2, ltc::get_type(static_cast<ltc::Type>(nType)), ltc::timecode::TYPE_MAX_LENGTH);

	if (m_bCursorOn) {
		Display::Get()->SetCursor(display::cursor::ON);
		Display::Get()->SetCursorPos(0, 1);
	} else {
		Display::Get()->SetCursor(display::cursor::OFF);
	}
}
