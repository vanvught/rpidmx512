/**
 * @file displayedittimecode.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "displayset.h"
#include "ltc.h"
#include "timecodeconst.h"
#include "input.h"
#include "display.h"
#include "firmware/debug/debug_debug.h"

static constexpr uint8_t kIndex[] = {ltc::timecode::index::HOURS_TENS,    ltc::timecode::index::HOURS_UNITS,  ltc::timecode::index::MINUTES_TENS,
                                     ltc::timecode::index::MINUTES_UNITS, ltc::timecode::index::SECONDS_TENS, ltc::timecode::index::SECONDS_UNITS,
                                     ltc::timecode::index::FRAMES_TENS,   ltc::timecode::index::FRAMES_UNITS};

static constexpr auto kIndexSize = sizeof(kIndex) / sizeof(kIndex[0]);

void DisplayEditTimeCode::HandleKey(int key, struct ltc::TimeCode& timecode, char time_code[ltc::timecode::CODE_MAX_LENGTH])
{
    DEBUG_PRINTF("%d %d", state_, key);

    m_nFrames = TimeCodeConst::FPS[timecode.type];

    if (state_ == IDLE)
    {
        if (key == input::KEY_ENTER)
        {
            state_ = EDIT;
            m_nCursorPositionIndex = 0;
            cursor_on_ = true;
        }
    }
    else
    {
        switch (key)
        {
            case input::KEY_ENTER:
                break;
            case input::KEY_ESC:
                state_ = IDLE;
                m_nCursorPositionIndex = 0;
                cursor_on_ = false;
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

    const auto* pTimeCode = &timecode;
    ltc::itoa_base10(pTimeCode, time_code);
    Display::Get()->TextLine(1, time_code, ltc::timecode::CODE_MAX_LENGTH);

    if (cursor_on_)
    {
        Display::Get()->SetCursor(display::cursor::kOn);
    }
    else
    {
        Display::Get()->SetCursor(display::cursor::kOff);
    }

    Display::Get()->SetCursorPos(kIndex[m_nCursorPositionIndex], 0);
}

void DisplayEditTimeCode::KeyUp(struct ltc::TimeCode& timecode)
{
    switch (kIndex[m_nCursorPositionIndex])
    {
        case ltc::timecode::index::HOURS_TENS:
            if (timecode.hours < 20)
            {
                timecode.hours = static_cast<uint8_t>(timecode.hours + 10U);
            }
            break;
        case ltc::timecode::index::HOURS_UNITS:
            if (timecode.hours < 23)
            {
                timecode.hours++;
                return;
            }
            timecode.hours = 0;
            break;
        case ltc::timecode::index::MINUTES_TENS:
            if (timecode.minutes < 50)
            {
                timecode.minutes = static_cast<uint8_t>(timecode.minutes + 10U);
            }
            break;
        case ltc::timecode::index::MINUTES_UNITS:
            if (timecode.minutes < 59)
            {
                timecode.minutes++;
                return;
            }
            timecode.minutes = 0;
            break;
        case ltc::timecode::index::SECONDS_TENS:
            if (timecode.seconds < 50)
            {
                timecode.seconds = static_cast<uint8_t>(timecode.seconds + 10U);
            }
            break;
        case ltc::timecode::index::SECONDS_UNITS:
            if (timecode.seconds < 59)
            {
                timecode.seconds++;
                return;
            }
            timecode.seconds = 0;
            break;
        case ltc::timecode::index::FRAMES_TENS:
            if (timecode.frames < ((m_nFrames / 10) * 10))
            {
                timecode.frames = static_cast<uint8_t>(timecode.frames + 10U);
            }
            break;
        case ltc::timecode::index::FRAMES_UNITS:
            if (timecode.frames < (m_nFrames - 1))
            {
                timecode.frames++;
                return;
            }
            timecode.frames = 0;
            break;
        default:
            break;
    }
}

void DisplayEditTimeCode::KeyDown(struct ltc::TimeCode& timecode)
{
    switch (kIndex[m_nCursorPositionIndex])
    {
        case ltc::timecode::index::HOURS_TENS:
            if (timecode.hours > 9)
            {
                timecode.hours = static_cast<uint8_t>(timecode.hours - 10U);
                return;
            }
            break;
        case ltc::timecode::index::HOURS_UNITS:
            if (timecode.hours > 0)
            {
                timecode.hours--;
                return;
            }
            timecode.hours = 23;
            break;
        case ltc::timecode::index::MINUTES_TENS:
            if (timecode.minutes > 9)
            {
                timecode.minutes = static_cast<uint8_t>(timecode.minutes - 10U);
            }
            break;
        case ltc::timecode::index::MINUTES_UNITS:
            if (timecode.minutes > 0)
            {
                timecode.minutes--;
                return;
            }
            timecode.minutes = 59;
            break;
        case ltc::timecode::index::SECONDS_TENS:
            if (timecode.seconds > 9)
            {
                timecode.seconds = static_cast<uint8_t>(timecode.seconds - 10U);
            }
            break;
        case ltc::timecode::index::SECONDS_UNITS:
            if (timecode.seconds > 0)
            {
                timecode.seconds--;
                return;
            }
            timecode.seconds = 59;
            break;
        case ltc::timecode::index::FRAMES_TENS:
            if (timecode.frames > 9)
            {
                timecode.frames = static_cast<uint8_t>(timecode.frames - 10U);
            }
            break;
        case ltc::timecode::index::FRAMES_UNITS:
            if (timecode.frames > 0)
            {
                timecode.frames--;
                return;
            }
            timecode.frames = static_cast<uint8_t>(m_nFrames - 1U);
            break;
        default:
            break;
    }
}

void DisplayEditTimeCode::KeyLeft()
{
    if (m_nCursorPositionIndex > 0)
    {
        m_nCursorPositionIndex--;
        return;
    }
    m_nCursorPositionIndex = kIndexSize - 1;
}

void DisplayEditTimeCode::KeyRight()
{
    if (m_nCursorPositionIndex < (kIndexSize - 1))
    {
        m_nCursorPositionIndex++;
        return;
    }
    m_nCursorPositionIndex = 0;
}
