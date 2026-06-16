/**
 * @file rgbpanel.cpp
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

#include <cassert>
#include <cstdint>
#include <cstdio>

#include "rgbpanel.h"
#include "../../lib-device/src/font_cp437.h"

RgbPanel::RgbPanel(uint32_t columns, uint32_t rows, uint32_t chain, rgbpanel::Types type)
    : columns_(columns),
      rows_(rows),
      chain_(chain != 0 ? chain : 1),
      type_(type),
      // Text
      max_position_(columns / kFontCP437CharW),
      max_line_(rows / kFontCP437CharH) {
    assert(s_this == nullptr);
    s_this = this;

    PlatformInit();

    // Text
    assert(columns % kFontCP437CharW == 0);
    assert(rows % kFontCP437CharH == 0);

    colons_ = new struct TColon[max_position_ * max_line_];
    assert(colons_ != nullptr);
    SetColonsOff();
}

/**
 * Text
 */
void RgbPanel::PutChar(char ch, uint8_t red, uint8_t green, uint8_t blue) {
    if (__builtin_expect((static_cast<uint32_t>(ch) >= Cp437FontSize()), 0)) {
        ch = ' ';
    }

    const auto kStartColumn = static_cast<uint8_t>(position_ * kFontCP437CharW);
    auto row = line_ * max_position_;
    const auto kColonIndex = position_ + row;
    const auto kShowColon = (colons_[kColonIndex].bits != 0);

    for (uint32_t i = 0; i < kFontCP437CharH; i++) {
        uint32_t width = 0;

        for (uint32_t column = kStartColumn; column < (kFontCP437CharW + kStartColumn); column++) {
            if ((kShowColon) && (column == (kStartColumn + kFontCP437CharW - 1U))) {
                const auto kByte = static_cast<uint8_t>(colons_[kColonIndex].bits >> i);

                if ((kByte & 0x1) != 0) {
                    SetPixel(column, row, colons_[kColonIndex].red, colons_[kColonIndex].green, colons_[kColonIndex].blue);
                } else {
                    SetPixel(column, row, 0, 0, 0);
                }

                continue;
            }

            const auto kByte = kCp437Font[static_cast<int>(ch)][width++] >> i;

            if ((kByte & 0x1) != 0) {
                SetPixel(column, row, red, green, blue);
            } else {
                SetPixel(column, row, 0, 0, 0);
            }
        }

        row++;
    }

    position_++;

    if (position_ == max_position_) {
        position_ = 0;
        line_++;

        if (line_ == max_line_) {
            line_ = 0;
        }
    }
}

void RgbPanel::PutString(const char* string, uint8_t red, uint8_t green, uint8_t blue) {
    char ch;

    while ((ch = *string++) != 0) {
        PutChar(ch, red, green, blue);
    }
}

void RgbPanel::Text(const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue) {
    if (__builtin_expect((length > max_position_), 0)) {
        length = max_position_;
    }

    for (uint32_t i = 0; i < length; i++) {
        PutChar(text[i], red, green, blue);
    }
}

/**
 * 1 is top line
 */
void RgbPanel::TextLine(uint8_t line, const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue) {
    if (__builtin_expect(((line == 0) || (line > max_line_)), 0)) {
        return;
    }

    SetCursorPos(0, static_cast<uint8_t>(line - 1));
    Text(text, length, red, green, blue);
}

/**
 * 1 is top line
 */
void RgbPanel::ClearLine(uint8_t line) {
    if (__builtin_expect(((line == 0) || (line > max_line_)), 0)) {
        return;
    }

    const auto kStartRow = (line - 1U) * max_position_;

    for (uint32_t row = kStartRow; row < (kStartRow + kFontCP437CharH); row++) {
        for (uint32_t column = 0; column < columns_; column++) {
            SetPixel(column, row, 0, 0, 0);
        }
    }

    SetCursorPos(0, static_cast<uint8_t>(line - 1));
}

/**
 * 0,0 is top left
 */
void RgbPanel::SetCursorPos(uint8_t column, uint8_t row) {
    if (__builtin_expect(((column >= max_position_) || (row >= max_line_)), 0)) {
        return;
    }

    position_ = column;
    line_ = row;
}

void RgbPanel::SetColon(char ch, uint8_t column, uint8_t row, uint8_t red, uint8_t green, uint8_t blue) {
    if (__builtin_expect(((column >= max_position_) || (row >= max_line_)), 0)) {
        return;
    }

    const auto kIndex = column + (row * max_position_);

    switch (ch) {
        case ':':
        case ';':
            colons_[kIndex].bits = 0x66;
            break;
        case '.':
        case ',':
            colons_[kIndex].bits = 0x60;
            break;
        default:
            colons_[kIndex].bits = 0;
            break;
    }

    colons_[kIndex].red = red;
    colons_[kIndex].blue = blue;
    colons_[kIndex].green = green;
}

void RgbPanel::SetColonsOff() {
    for (uint32_t index = 0; index < max_position_ * max_line_; index++) {
        colons_[index].bits = 0;
        colons_[index].red = 0;
        colons_[index].green = 0;
        colons_[index].blue = 0;
    }
}

/**
 * void RgbPanel::Start() is implemented as platform specific
 */

void RgbPanel::Stop() {
    if (!started_) {
        return;
    }

    started_ = false;

    Cls();
    Show();
}

void RgbPanel::Print() {
    puts("RGB led panel");
    printf(" %ux%ux%u\n", columns_, rows_, chain_);
}
