/**
 * @file displayhandler.h
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

#ifndef DISPLAYHANDLER_H_
#define DISPLAYHANDLER_H_

#include "display.h"
#include "hal_statusled.h"

namespace hal
{
void DisplayStatusled(hal::statusled::Mode status_led_mode)
{
    if (Display::Get()->IsDetected())
    {
        char c;
        switch (status_led_mode)
        {
            case hal::statusled::Mode::OFF_OFF:
                c = 'O';
                break;
            case hal::statusled::Mode::OFF_ON:
                c = 'O';
                break;
            case hal::statusled::Mode::NORMAL:
                c = 'N';
                break;
            case hal::statusled::Mode::DATA:
                c = 'D';
                break;
            case hal::statusled::Mode::FAST:
                c = 'F';
                break;
            case hal::statusled::Mode::REBOOT:
                c = 'R';
                break;
            default:
                c = 'U';
                break;
        }

        Display::Get()->SetCursorPos(Display::Get()->GetColumns() - 1U, Display::Get()->GetRows() - 1U);
        Display::Get()->PutChar(c);
    }
}
} // namespace hal

#endif  // DISPLAYHANDLER_H_
