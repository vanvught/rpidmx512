/**
 * @file handleroled.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef COMMON_FIRMWARE_JAMSTAPL_HANDLEROLED_H_
#define COMMON_FIRMWARE_JAMSTAPL_HANDLEROLED_H_

#include "jamstapl.h"
#include "console.h"
#include "display.h"

struct HandlerOled : public JamSTAPLDisplay
{
    HandlerOled() { s_this = this; }
    ~HandlerOled() override = default;

    // JamSTAPL
    void JamShowInfo(const char* info) override
    {
        Display::Get()->ClearLine(1);
        Display::Get()->Write(1, info);
    }

    void JamShowStatus(const char* status, int exit_code) override
    {
        Display::Get()->TextStatus(status, exit_code == 0 ? console::Colours::kConsoleGreen : console::Colours::kConsoleRed);
    }

    static HandlerOled* Get() { return s_this; }

   private:
    inline static HandlerOled* s_this;
};

#endif  // COMMON_FIRMWARE_JAMSTAPL_HANDLEROLED_H_
