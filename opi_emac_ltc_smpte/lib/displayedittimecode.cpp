/**
 * @file displayedittimecode.cpp
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

#include <stdio.h>

#include "displayedittimecode.h"

#include "ltc.h"
#include "timecodeconst.h"
#include "input.h"
#include "display.h"

static constexpr TLtcTimeCodeIndex s_Index[] = {
		LTC_TC_INDEX_HOURS_TENS,
		LTC_TC_INDEX_HOURS_UNITS,
		LTC_TC_INDEX_MINUTES_TENS,
		LTC_TC_INDEX_MINUTES_UNITS,
		LTC_TC_INDEX_SECONDS_TENS,
		LTC_TC_INDEX_SECONDS_UNITS,
		LTC_TC_INDEX_FRAMES_TENS,
		LTC_TC_INDEX_FRAMES_UNITS
};

static constexpr auto s_IndexSize = sizeof(s_Index) / sizeof(s_Index[0]);

void DisplayEditTimeCode::HandleKey(int nKey, TLtcTimeCode &timecode, char m_aTimeCode[TC_CODE_MAX_LENGTH]) {
	m_nFrames = TimeCodeConst::FPS[timecode.nType];

	if (m_State == IDLE) {
		if (nKey == INPUT_KEY_ENTER) {
			m_State = EDIT;
			m_nCursorPositionIndex = 0;
			m_bCursorOn = true;
		}
	} else {
		switch (nKey) {
		case INPUT_KEY_ESC:
			m_State = IDLE;
			m_nCursorPositionIndex = 0;
			m_bCursorOn = false;
			break;
		case INPUT_KEY_UP:
			KeyUp(timecode);
			break;
		case INPUT_KEY_DOWN:
			KeyDown(timecode);
			break;
		case INPUT_KEY_LEFT:
			KeyLeft();
			break;
		case INPUT_KEY_RIGHT:
			KeyRight();
			break;
		default:
			break;
		}
	}

	const TLtcTimeCode *pTimeCode = &timecode;
	Ltc::ItoaBase10(pTimeCode, m_aTimeCode);
	Display::Get()->TextLine(1, m_aTimeCode, TC_CODE_MAX_LENGTH);

	if (m_bCursorOn) {
		Display::Get()->SetCursor(display::cursor::ON);
	} else {
		Display::Get()->SetCursor(display::cursor::OFF);
	}

	Display::Get()->SetCursorPos(s_Index[m_nCursorPositionIndex], 0);

	printf("m_State=%d, s_Index[m_nCursorPositionIndex]=%d, m_bCursorOn=%d\n", m_State, s_Index[m_nCursorPositionIndex], m_bCursorOn);
	printf("%.2d:%.2d:%.2d.%.2d\n", timecode.nHours, timecode.nMinutes, timecode.nSeconds, timecode.nFrames);
}

void DisplayEditTimeCode::KeyUp(TLtcTimeCode& timecode) {
	switch (s_Index[m_nCursorPositionIndex]) {
		case LTC_TC_INDEX_HOURS_TENS:
			if (timecode.nHours < 20) {
				timecode.nHours += 10;
			}
			break;
		case LTC_TC_INDEX_HOURS_UNITS:
			if (timecode.nHours < 23) {
				timecode.nHours += 1;
				return;
			}
			timecode.nHours = 0;
			break;
		case LTC_TC_INDEX_MINUTES_TENS:
			if (timecode.nMinutes < 50) {
				timecode.nMinutes += 10;
			}
			break;
		case LTC_TC_INDEX_MINUTES_UNITS:
			if (timecode.nMinutes < 59) {
				timecode.nMinutes += 1;
				return;
			}
			timecode.nMinutes = 0;
			break;
		case LTC_TC_INDEX_SECONDS_TENS:
			if (timecode.nSeconds < 50) {
				timecode.nSeconds += 10;
			}
			break;
		case LTC_TC_INDEX_SECONDS_UNITS:
			if (timecode.nSeconds < 59) {
				timecode.nSeconds += 1;
				return;
			}
			timecode.nSeconds = 0;
			break;
		case LTC_TC_INDEX_FRAMES_TENS:
			if (timecode.nFrames < ((m_nFrames / 10) * 10)) {
				timecode.nFrames += 10;
			}
			break;
		case LTC_TC_INDEX_FRAMES_UNITS:
			if (timecode.nFrames < (m_nFrames - 1)) {
				timecode.nFrames += 1;
				return;
			}
			timecode.nFrames = 0;
			break;
		default:
			break;
	}
}

void DisplayEditTimeCode::KeyDown(TLtcTimeCode& timecode) {
	switch (s_Index[m_nCursorPositionIndex]) {
		case LTC_TC_INDEX_HOURS_TENS:
			if (timecode.nHours > 9) {
				timecode.nHours -= 10;
				return;
			}
			break;
		case LTC_TC_INDEX_HOURS_UNITS:
			if (timecode.nHours > 0) {
				timecode.nHours -= 1;
				return;
			}
			timecode.nHours = 23;
			break;
		case LTC_TC_INDEX_MINUTES_TENS:
			if (timecode.nMinutes > 9) {
				timecode.nMinutes -= 10;
			}
			break;
		case LTC_TC_INDEX_MINUTES_UNITS:
			if (timecode.nMinutes > 0) {
				timecode.nMinutes -= 1;
				return;
			}
			timecode.nMinutes = 59;
			break;
		case LTC_TC_INDEX_SECONDS_TENS:
			if (timecode.nSeconds > 9) {
				timecode.nSeconds -= 10;
			}
			break;
		case LTC_TC_INDEX_SECONDS_UNITS:
			if (timecode.nSeconds > 0) {
				timecode.nSeconds -= 1;
				return;
			}
			timecode.nSeconds = 59;
			break;
		case LTC_TC_INDEX_FRAMES_TENS:
			if (timecode.nFrames > 9) {
				timecode.nFrames -= 10;
			}
			break;
		case LTC_TC_INDEX_FRAMES_UNITS:
			if (timecode.nFrames > 0) {
				timecode.nFrames -= 1;
				return;
			}
			timecode.nFrames = m_nFrames - 1;
			break;
		default:
			break;
	}
}

void DisplayEditTimeCode::KeyLeft() {
	if (m_nCursorPositionIndex > 0) {
		m_nCursorPositionIndex--;
		return;
	}
	m_nCursorPositionIndex = s_IndexSize - 1;
}

void DisplayEditTimeCode::KeyRight() {
	if (m_nCursorPositionIndex < s_IndexSize) {
		m_nCursorPositionIndex++;
		return;
	}
	m_nCursorPositionIndex = 0;
}
