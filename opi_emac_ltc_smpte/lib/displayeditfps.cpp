/**
 * @file displayeditfps.cpp
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

#include "displayeditfps.h"
#include "displayset.h"
#include "input.h"
#include "ltc.h"
#include "display.h"
 #include "firmware/debug/debug_debug.h"

static void KeyLeft(uint8_t& type)
{
    if (type > 0)
    {
        type--;
        return;
    }
    type = 3;
}

static void KeyRight(uint8_t& type)
{
    if (type < 3)
    {
        type++;
        return;
    }
    type = 0;
}

void DisplayEditFps::HandleKey(int key, uint8_t& type)
{
    DEBUG_PRINTF("%d %d", state_, key);

    if (state_ == IDLE)
    {
        if (key == input::KEY_ENTER)
        {
            state_ = EDIT;
            m_bCursorOn = true;
        }
    }
    else
    {
        switch (key)
        {
            case input::KEY_ESC:
                state_ = IDLE;
                m_bCursorOn = false;
                break;
            case input::KEY_DOWN:
            case input::KEY_LEFT:
                KeyLeft(type);
                break;
            case input::KEY_UP:
            case input::KEY_RIGHT:
                KeyRight(type);
                break;
            default:
                break;
        }
    }

    Display::Get()->TextLine(2, ltc::get_type(static_cast<ltc::Type>(type)), ltc::timecode::TYPE_MAX_LENGTH);

    if (m_bCursorOn)
    {
        Display::Get()->SetCursor(display::cursor::kOn);
        Display::Get()->SetCursorPos(0, 1);
    }
    else
    {
        Display::Get()->SetCursor(display::cursor::kOff);
    }
}
