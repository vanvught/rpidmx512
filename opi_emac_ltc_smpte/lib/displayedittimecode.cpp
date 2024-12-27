/**
 * @file displayedittimecode.cpp
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

#include "displayedittimecode.h"

#include "ltc.h"
#include "timecodeconst.h"
#include "input.h"
#include "display.h"

#include "debug.h"

static constexpr uint8_t s_Index[] = {
		ltc::timecode::index::HOURS_TENS,
		ltc::timecode::index::HOURS_UNITS,
		ltc::timecode::index::MINUTES_TENS,
		ltc::timecode::index::MINUTES_UNITS,
		ltc::timecode::index::SECONDS_TENS,
		ltc::timecode::index::SECONDS_UNITS,
		ltc::timecode::index::FRAMES_TENS,
		ltc::timecode::index::FRAMES_UNITS
};

static constexpr auto s_IndexSize = sizeof(s_Index) / sizeof(s_Index[0]);

void DisplayEditTimeCode::HandleKey(int nKey, struct ltc::TimeCode& timecode, char m_aTimeCode[ltc::timecode::CODE_MAX_LENGTH]) {
	DEBUG_PRINTF("%d %d", m_State, nKey);

	m_nFrames = TimeCodeConst::FPS[timecode.nType];

	if (m_State == IDLE) {
		if (nKey == input::KEY_ENTER) {
			m_State = EDIT;
			m_nCursorPositionIndex = 0;
			m_bCursorOn = true;
		}
	} else {
		switch (nKey) {
		case input::KEY_ENTER:
			break;
		case input::KEY_ESC:
			m_State = IDLE;
			m_nCursorPositionIndex = 0;
			m_bCursorOn = false;
			break;
		case input::KEY_UP:
			KeyUp(timecode);
			break;
		case input::KEY_DOWN:
			KeyDown(timecode);
			break;
		case input::KEY_LEFT:
			KeyLeft();
			break;
		case input::KEY_RIGHT:
			KeyRight();
			break;
		default:
			return;
			break;
		}
	}

	const auto *pTimeCode = &timecode;
	ltc::itoa_base10(pTimeCode, m_aTimeCode);
	Display::Get()->TextLine(1, m_aTimeCode, ltc::timecode::CODE_MAX_LENGTH);

	if (m_bCursorOn) {
		Display::Get()->SetCursor(display::cursor::ON);
	} else {
		Display::Get()->SetCursor(display::cursor::OFF);
	}

	Display::Get()->SetCursorPos(s_Index[m_nCursorPositionIndex], 0);
}

void DisplayEditTimeCode::KeyUp(struct ltc::TimeCode& timecode) {
	switch (s_Index[m_nCursorPositionIndex]) {
		case ltc::timecode::index::HOURS_TENS:
			if (timecode.nHours < 20) {
				timecode.nHours = static_cast<uint8_t>(timecode.nHours + 10U);
			}
			break;
		case ltc::timecode::index::HOURS_UNITS:
			if (timecode.nHours < 23) {
				timecode.nHours++;
				return;
			}
			timecode.nHours = 0;
			break;
		case ltc::timecode::index::MINUTES_TENS:
			if (timecode.nMinutes < 50) {
				timecode.nMinutes = static_cast<uint8_t>(timecode.nMinutes + 10U);
			}
			break;
		case ltc::timecode::index::MINUTES_UNITS:
			if (timecode.nMinutes < 59) {
				timecode.nMinutes++;
				return;
			}
			timecode.nMinutes = 0;
			break;
		case ltc::timecode::index::SECONDS_TENS:
			if (timecode.nSeconds < 50) {
				timecode.nSeconds = static_cast<uint8_t>(timecode.nSeconds + 10U);
			}
			break;
		case ltc::timecode::index::SECONDS_UNITS:
			if (timecode.nSeconds < 59) {
				timecode.nSeconds++;
				return;
			}
			timecode.nSeconds = 0;
			break;
		case ltc::timecode::index::FRAMES_TENS:
			if (timecode.nFrames < ((m_nFrames / 10) * 10)) {
				timecode.nFrames = static_cast<uint8_t>(timecode.nFrames + 10U);
			}
			break;
		case ltc::timecode::index::FRAMES_UNITS:
			if (timecode.nFrames < (m_nFrames - 1)) {
				timecode.nFrames++;
				return;
			}
			timecode.nFrames = 0;
			break;
		default:
			break;
	}
}

void DisplayEditTimeCode::KeyDown(struct ltc::TimeCode& timecode) {
	switch (s_Index[m_nCursorPositionIndex]) {
		case ltc::timecode::index::HOURS_TENS:
			if (timecode.nHours > 9) {
				timecode.nHours = static_cast<uint8_t>(timecode.nHours - 10U);
				return;
			}
			break;
		case ltc::timecode::index::HOURS_UNITS:
			if (timecode.nHours > 0) {
				timecode.nHours--;
				return;
			}
			timecode.nHours = 23;
			break;
		case ltc::timecode::index::MINUTES_TENS:
			if (timecode.nMinutes > 9) {
				timecode.nMinutes = static_cast<uint8_t>(timecode.nMinutes - 10U);
			}
			break;
		case ltc::timecode::index::MINUTES_UNITS:
			if (timecode.nMinutes > 0) {
				timecode.nMinutes--;
				return;
			}
			timecode.nMinutes = 59;
			break;
		case ltc::timecode::index::SECONDS_TENS:
			if (timecode.nSeconds > 9) {
				timecode.nSeconds = static_cast<uint8_t>(timecode.nSeconds - 10U);
			}
			break;
		case ltc::timecode::index::SECONDS_UNITS:
			if (timecode.nSeconds > 0) {
				timecode.nSeconds--;
				return;
			}
			timecode.nSeconds = 59;
			break;
		case ltc::timecode::index::FRAMES_TENS:
			if (timecode.nFrames > 9) {
				timecode.nFrames = static_cast<uint8_t>(timecode.nFrames - 10U);
			}
			break;
		case ltc::timecode::index::FRAMES_UNITS:
			if (timecode.nFrames > 0) {
				timecode.nFrames--;
				return;
			}
			timecode.nFrames = static_cast<uint8_t>(m_nFrames - 1U);
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
	if (m_nCursorPositionIndex < (s_IndexSize - 1)) {
		m_nCursorPositionIndex++;
		return;
	}
	m_nCursorPositionIndex = 0;
}
