/**
 * @file showfiledisplay.cpp
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "showfile.h"
#include "showfiledisplay.h"

#include "display.h"

namespace showfile
{
void DisplayFilename(const char* file_name, [[maybe_unused]] int32_t show)
{
    assert(file_name != nullptr);

    if (file_name[0] != 0)
    {
        Display::Get()->TextStatus(file_name);
    }
    else
    {
        Display::Get()->TextStatus("No showfile");
    }
}

void DisplayStatus()
{
    Display::Get()->SetCursorPos(0, 6);

    switch (ShowFile::Instance().GetStatus())
    {
        case showfile::Status::kIdle:
            Display::Get()->PutString("Idle     ");
            break;
        case showfile::Status::kPlaying:
            Display::Get()->PutString("Running  ");
            break;
        case showfile::Status::kStopped:
            Display::Get()->PutString("Stopped  ");
            break;
        case showfile::Status::kEnded:
            Display::Get()->PutString("Ended    ");
            break;
        case showfile::Status::kRecording:
            Display::Get()->PutString("Recording ");
            break;
        case showfile::Status::kUndefined:
        default:
            Display::Get()->PutString("No Status");
            break;
    }

    Display::Get()->SetCursorPos(11, 7);

    if (ShowFile::Instance().IsTFTPEnabled())
    {
        Display::Get()->PutString("[TFTP On]");
    }
    else if (ShowFile::Instance().GetDoLoop())
    {
        Display::Get()->PutString("[Looping]");
    }
    else
    {
        Display::Get()->PutString("         ");
    }
}
} // namespace showfile
