/**
 * @file displayeditfps.cpp
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "display.h"
#include "input.h"
#include "ltc.h"

void DisplayEditFps::HandleKey(int key, uint8_t& type)
{
    if (state_ == kIdle)
    {
        if (key == input::KEY_ENTER)
        {
            state_ = kEdit;
            cursor_on_ = true;
        }
    }
    else
    {
        switch (key)
        {
            case input::KEY_ESC:
                state_ = kIdle;
                cursor_on_ = false;
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

    if (cursor_on_)
    {
        Display::Get()->SetCursor(display::cursor::kOn);
        Display::Get()->SetCursorPos(0, 1);
    }
    else
    {
        Display::Get()->SetCursor(display::cursor::kOff);
    }
}

void DisplayEditFps::KeyLeft(uint8_t& type)
{
    if (type > 0)
    {
        type--;
        return;
    }
    type = 3;
}

void DisplayEditFps::KeyRight(uint8_t& type)
{
    if (type < 3)
    {
        type++;
        return;
    }
    type = 0;
}