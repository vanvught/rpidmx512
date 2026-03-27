/**
 * @file ws28xxdisplaymatrix.cpp
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
    : columns_(columns),
      rows_(rows),
      m_nOffset(static_cast<uint32_t>(rows - FONT_CP437_CHAR_H) * 2U),
      m_nMaxLeds(static_cast<uint8_t>(columns * rows)),
      max_position_(columns / FONT_CP437_CHAR_W),
      max_line_(rows / FONT_CP437_CHAR_H)
{
    DEBUG_ENTRY();

    assert(columns % FONT_CP437_CHAR_W == 0);
    assert(rows % FONT_CP437_CHAR_H == 0);

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

    DEBUG_PRINTF("columns_=%u, rows_=%u, m_nOffset=%u, max_position_=%u, max_line_=%u", columns_, rows_, m_nOffset, max_position_, max_line_);
    DEBUG_EXIT();
}

WS28xxDisplayMatrix::~WS28xxDisplayMatrix()
{
    DEBUG_ENTRY();

    if (m_pPixelOutput != nullptr)
    {
        Cls();

        delete m_pPixelOutput;
        m_pPixelOutput = nullptr;
    }

    delete[] colons_;
    colons_ = nullptr;

    DEBUG_EXIT();
}

void WS28xxDisplayMatrix::PutChar(char nChar, uint8_t red, uint8_t green, uint8_t blue)
{
    if (static_cast<uint32_t>(nChar) >= cp437_font_size())
    {
        nChar = ' ';
    }

    while (m_pPixelOutput->IsUpdating())
    {
        // wait for completion
    }

    auto nOffset = static_cast<uint16_t>((FONT_CP437_CHAR_W * FONT_CP437_CHAR_H) * position_);

    for (uint32_t nWidth = 0; nWidth < FONT_CP437_CHAR_W; nWidth++)
    {
        uint8_t nByte = cp437_font[static_cast<int>(nChar)][nWidth];

        // FIXME This can be optimized. See rgbpanel code

        if (nWidth == (FONT_CP437_CHAR_W - 1))
        {
            if (colons_[position_].bits != 0)
            {
                nByte = colons_[position_].bits;
                red = colons_[position_].red;
                green = colons_[position_].green;
                blue = colons_[position_].blue;
            }
        }

        if ((nWidth & 0x1) != 0)
        {
            nByte = ReverseBits(nByte);
        }

        for (uint32_t nHeight = 0; nHeight < FONT_CP437_CHAR_H; nHeight++)
        {
            if (nByte & (1 << nHeight))
            {
                m_pPixelOutput->SetPixel(nOffset, red, green, blue);
            }
            else
            {
                m_pPixelOutput->SetPixel(nOffset, 0x00, 0x00, 0x00);
            }

            nOffset++;
        }
    }

    position_++;

    if (position_ == max_position_)
    {
        position_ = 0;
        line_++;

        if (line_ == max_line_)
        {
            line_ = 0;
        }
    }

    m_bUpdateNeeded = true;
}

void WS28xxDisplayMatrix::PutString(const char* pString, uint8_t red, uint8_t green, uint8_t blue)
{
    char nChar;

    while ((nChar = *pString++) != 0)
    {
        PutChar(nChar, red, green, blue);
    }
}

void WS28xxDisplayMatrix::Text(const char* pText, uint32_t nLength, uint8_t red, uint8_t green, uint8_t blue)
{
    if (nLength > max_position_)
    {
        nLength = max_position_;
    }

    for (uint32_t i = 0; i < nLength; i++)
    {
        PutChar(pText[i], red, green, blue);
    }
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::TextLine(uint8_t nLine, const char* pText, uint32_t nLength, uint8_t red, uint8_t green, uint8_t blue)
{
    if ((nLine == 0) || (nLine > max_line_))
    {
        return;
    }

    SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
    Text(pText, nLength, red, green, blue);
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::ClearLine(uint8_t nLine)
{
    if ((nLine == 0) || (nLine > max_line_))
    {
        return;
    }

    while (m_pPixelOutput->IsUpdating())
    {
        // wait for completion
    }

    for (uint32_t i = 0; i < m_nMaxLeds; i++)
    {
        m_pPixelOutput->SetPixel(i, 0, 0, 0); ///< Note: Currently working for single row only
    }

    SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
}

/**
 * 0,0 is top left
 */
void WS28xxDisplayMatrix::SetCursorPos(uint8_t nCol, uint8_t row)
{
    if ((nCol >= max_position_) || (row >= max_line_))
    {
        return;
    }

    position_ = nCol;
    line_ = row;
}

void WS28xxDisplayMatrix::Cls()
{
    while (m_pPixelOutput->IsUpdating())
    {
        // wait for completion
    }
    m_pPixelOutput->Blackout();
}

void WS28xxDisplayMatrix::Show()
{
    if (m_bUpdateNeeded)
    {
        m_bUpdateNeeded = false;
        m_pPixelOutput->Update();
    }
}

void WS28xxDisplayMatrix::SetColon(char nChar, uint32_t nPos, uint8_t red, uint8_t green, uint8_t blue)
{
    if (nPos >= max_position_)
    {
        return;
    }

    switch (nChar)
    {
        case ':':
            colons_[nPos].bits = 0x66;
            break;
        case '.':
            colons_[nPos].bits = 0x60;
            break;
        default:
            colons_[nPos].bits = 0;
            break;
    }

    colons_[nPos].red = red;
    colons_[nPos].blue = blue;
    colons_[nPos].green = green;
}

void WS28xxDisplayMatrix::SetColonsOff()
{
    for (uint32_t nPos = 0; nPos < max_position_; nPos++)
    {
        colons_[nPos].bits = 0;
        colons_[nPos].red = 0;
        colons_[nPos].green = 0;
        colons_[nPos].blue = 0;
    }
}

uint8_t WS28xxDisplayMatrix::ReverseBits(uint8_t bits)
{
#if defined(H3)
    const auto input = static_cast<uint32_t>(bits);
    uint32_t output;
    asm("rbit %0, %1" : "=r"(output) : "r"(input));
    return static_cast<uint8_t>((output >> 24));
#else
    // http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
    const uint8_t nResult = ((bits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
    return nResult;
#endif
}
