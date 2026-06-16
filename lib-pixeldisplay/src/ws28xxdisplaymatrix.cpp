/**
 * @file ws28xxdisplaymatrix.cpp
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cassert>

#include "pixeldisplaymatrix.h"
#include "pixelconfiguration.h"
#include "pixeltype.h"
#include "pixeloutput.h"
#include "../../lib-device/src/font_cp437.h"
#include "firmware/debug/debug_debug.h"

using namespace pixel;

///< Note: Currently working for single row only

WS28xxDisplayMatrix::WS28xxDisplayMatrix(uint8_t columns, uint8_t rows, LedType led_type, LedMap led_map)
    : columns_(columns), rows_(rows), offset_((rows - kFontCP437CharH) * 2U), m_nMaxLeds(static_cast<uint8_t>(columns * rows)), max_position_(columns / kFontCP437CharW), max_line_(rows / kFontCP437CharH) {
    DEBUG_ENTRY();

    assert(columns % kFontCP437CharW == 0);
    assert(rows % kFontCP437CharH == 0);

    colons_ = new struct TWS28xxDisplayMatrixColon[max_position_];
    assert(colons_ != nullptr);

    SetColonsOff();

    PixelConfiguration pixel_configuration;

    pixel_configuration.SetType(led_type);
    pixel_configuration.SetMap(led_map);

    pixel_configuration.SetCount(m_nMaxLeds);

    m_pPixelOutput = new PixelOutput;
    assert(m_pPixelOutput != nullptr);
    m_pPixelOutput->Blackout();

    DEBUG_PRINTF("columns_=%u, rows_=%u, offset_=%u, max_position_=%u, max_line_=%u", columns_, rows_, offset_, max_position_, max_line_);
    DEBUG_EXIT();
}

WS28xxDisplayMatrix::~WS28xxDisplayMatrix() {
    DEBUG_ENTRY();

    if (m_pPixelOutput != nullptr) {
        Cls();

        delete m_pPixelOutput;
        m_pPixelOutput = nullptr;
    }

    delete[] colons_;
    colons_ = nullptr;

    DEBUG_EXIT();
}

void WS28xxDisplayMatrix::PutChar(char ch, uint8_t red, uint8_t green, uint8_t blue) {
    if (static_cast<uint32_t>(ch) >= Cp437FontSize()) {
        ch = ' ';
    }

    while (m_pPixelOutput->IsUpdating()) {
        // wait for completion
    }

    auto offset = static_cast<uint16_t>((kFontCP437CharW * kFontCP437CharH) * position_);

    for (uint32_t width = 0; width < kFontCP437CharW; width++) {
        uint8_t byte = kCp437Font[static_cast<int>(ch)][width];

        // FIXME This can be optimized. See rgbpanel code

        if (width == (kFontCP437CharW - 1)) {
            if (colons_[position_].bits != 0) {
                byte = colons_[position_].bits;
                red = colons_[position_].red;
                green = colons_[position_].green;
                blue = colons_[position_].blue;
            }
        }

        if ((width & 0x1) != 0) {
            byte = ReverseBits(byte);
        }

        for (uint32_t height = 0; height < kFontCP437CharH; height++) {
            if (byte & (1 << height)) {
                m_pPixelOutput->SetPixel(offset, red, green, blue);
            } else {
                m_pPixelOutput->SetPixel(offset, 0x00, 0x00, 0x00);
            }

            offset++;
        }
    }

    position_++;

    if (position_ == max_position_) {
        position_ = 0;
        line_++;

        if (line_ == max_line_) {
            line_ = 0;
        }
    }

    m_bUpdateNeeded = true;
}

void WS28xxDisplayMatrix::PutString(const char* string, uint8_t red, uint8_t green, uint8_t blue) {
    char ch;

    while ((ch = *string++) != 0) {
        PutChar(ch, red, green, blue);
    }
}

void WS28xxDisplayMatrix::Text(const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue) {
    if (length > max_position_) {
        length = max_position_;
    }

    for (uint32_t i = 0; i < length; i++) {
        PutChar(text[i], red, green, blue);
    }
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::TextLine(uint8_t line, const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue) {
    if ((line == 0) || (line > max_line_)) {
        return;
    }

    SetCursorPos(0, static_cast<uint8_t>(line - 1));
    Text(text, length, red, green, blue);
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::ClearLine(uint8_t line) {
    if ((line == 0) || (line > max_line_)) {
        return;
    }

    while (m_pPixelOutput->IsUpdating()) {
        // wait for completion
    }

    for (uint32_t i = 0; i < m_nMaxLeds; i++) {
        m_pPixelOutput->SetPixel(i, 0, 0, 0); ///< Note: Currently working for single row only
    }

    SetCursorPos(0, static_cast<uint8_t>(line - 1));
}

/**
 * 0,0 is top left
 */
void WS28xxDisplayMatrix::SetCursorPos(uint8_t column, uint8_t row) {
    if ((column >= max_position_) || (row >= max_line_)) {
        return;
    }

    position_ = column;
    line_ = row;
}

void WS28xxDisplayMatrix::Cls() {
    while (m_pPixelOutput->IsUpdating()) {
        // wait for completion
    }
    m_pPixelOutput->Blackout();
}

void WS28xxDisplayMatrix::Show() {
    if (m_bUpdateNeeded) {
        m_bUpdateNeeded = false;
        m_pPixelOutput->Update();
    }
}

void WS28xxDisplayMatrix::SetColon(char ch, uint32_t position, uint8_t red, uint8_t green, uint8_t blue) {
    if (position >= max_position_) {
        return;
    }

    switch (ch) {
        case ':':
            colons_[position].bits = 0x66;
            break;
        case '.':
            colons_[position].bits = 0x60;
            break;
        default:
            colons_[position].bits = 0;
            break;
    }

    colons_[position].red = red;
    colons_[position].blue = blue;
    colons_[position].green = green;
}

void WS28xxDisplayMatrix::SetColonsOff() {
    for (uint32_t position = 0; position < max_position_; position++) {
        colons_[position].bits = 0;
        colons_[position].red = 0;
        colons_[position].green = 0;
        colons_[position].blue = 0;
    }
}

uint8_t WS28xxDisplayMatrix::ReverseBits(uint8_t bits) {
#if defined(H3)
    const auto kInput = static_cast<uint32_t>(bits);
    uint32_t output;
    asm("rbit %0, %1" : "=r"(output) : "r"(kInput));
    return static_cast<uint8_t>((output >> 24));
#else
    // http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
    const uint8_t kResult = ((bits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
    return kResult;
#endif
}
