/**
 * @file ansi_colour.h
 *
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ANSI_COLOUR_H_
#define ANSI_COLOUR_H_

namespace ansi {
// https://github.com/shiena/ansicolor/blob/master/README.md
struct Colours {
    enum class Colour { 
		kBlack, 
		kRed, 
		kGreen, 
		kYellow, 
		kBlue, 
		kMagenta, 
		kCyan, 
		kWhite, 
		kDefault
	};

    struct Fg {
        static constexpr char kBlack[] = "\x1b[30m";
        static constexpr char kRed[] = "\x1b[31m";
        static constexpr char kGreen[] = "\x1b[32m";
        static constexpr char kYellow[] = "\x1b[33m";
        static constexpr char kWhite[] = "\x1b[37m";
        static constexpr char kDefault[] = "\x1b[39m";
    };

    static constexpr const char* Foreground(Colour colour) {
        switch (colour) {
            case Colour::kBlack:
                return ansi::Colours::Fg::kBlack;
                break;
            case Colour::kRed:
                return ansi::Colours::Fg::kRed;
                break;
            case Colour::kGreen:
                return ansi::Colours::Fg::kGreen;
                break;
            case Colour::kYellow:
                return ansi::Colours::Fg::kYellow;
                break;
            case Colour::kWhite:
                return ansi::Colours::Fg::kWhite;
                break;
            default:
                return ansi::Colours::Fg::kDefault;
                break;
        }
    };

    struct Bg {
        static constexpr char kBlack[] = "\x1b[40m";
        static constexpr char kRed[] = "\x1b[41m";
        static constexpr char kGreen[] = "\x1b[42m";
        static constexpr char kYellow[] = "\x1b[43m";
        static constexpr char kWhite[] = "\x1b[47m";
        static constexpr char kDefault[] = "\x1b[49m";
    };

    static constexpr const char* Background(Colour colour) {
        switch (colour) {
            case Colour::kBlack:
                return ansi::Colours::Bg::kBlack;
                break;
            case Colour::kRed:
                return ansi::Colours::Bg::kRed;
                break;
            case Colour::kGreen:
                return ansi::Colours::Bg::kGreen;
                break;
            case Colour::kYellow:
                return ansi::Colours::Bg::kYellow;
                break;
            case Colour::kWhite:
                return ansi::Colours::Bg::kWhite;
                break;
            default:
                return ansi::Colours::Bg::kDefault;
                break;
        }
    };
};
} // namespace ansi

#endif // ANSI_COLOUR_H_
