/**
 * @file console.cpp
 *
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>

#include "console.h"
#include "ansi_colour.h"

namespace console {
void Status(ansi::Colours::Colour colour, const char* s) {
    const char* c;

    switch (colour) {
        case ansi::Colours::Colour::kBlack:
            c = ansi::Colours::Fg::kBlack;
            break;
        case ansi::Colours::Colour::kRed:
            c = ansi::Colours::Fg::kRed;
            break;
        case ansi::Colours::Colour::kGreen:
            c = ansi::Colours::Fg::kGreen;
            break;
        case ansi::Colours::Colour::kYellow:
            c = ansi::Colours::Fg::kYellow;
            break;
        case ansi::Colours::Colour::kWhite:
            c = ansi::Colours::Fg::kWhite;
            break;
        default:
            c = ansi::Colours::Fg::kDefault;
            break;
    }

    printf("%s[%s]%s\n", c, s, ansi::Colours::Fg::kDefault);
}

void Error(const char* s) {
    fprintf(stderr, "%s", s);
}
} // namespace console
