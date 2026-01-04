/**
 * @file ltcdisplaypixelmatrix.cpp
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#pragma GCC optimize("-funroll-loops")
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcdisplaypixelmatrix.h"
#include "pixeldisplaymatrix.h"
#include "pixeltype.h"
 #include "firmware/debug/debug_debug.h"

LtcDisplayPixelMatrix::LtcDisplayPixelMatrix(pixel::Type type, pixel::Map map)
{
    DEBUG_ENTRY();

    pixel_matrix_ = new WS28xxDisplayMatrix(64, 8, type, map);
    assert(pixel_matrix_ != nullptr);

    DEBUG_EXIT();
}

void LtcDisplayPixelMatrix::Show(const char* timecode, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons)
{
    pixel_matrix_->SetColonsOff();
    pixel_matrix_->SetColon(timecode[ltc::timecode::index::COLON_1], 1, colours_colons.red, colours_colons.green, colours_colons.blue);
    pixel_matrix_->SetColon(timecode[ltc::timecode::index::COLON_2], 3, colours_colons.red, colours_colons.green, colours_colons.blue);
    pixel_matrix_->SetColon(timecode[ltc::timecode::index::COLON_3], 5, colours_colons.red, colours_colons.green, colours_colons.blue);

    const char kLine[] = {timecode[0], timecode[1], timecode[3], timecode[4], timecode[6], timecode[7], timecode[9], timecode[10]};

    pixel_matrix_->TextLine(1, kLine, sizeof(kLine), colours.red, colours.green, colours.blue);
    pixel_matrix_->Show();
}

void LtcDisplayPixelMatrix::ShowSysTime(const char* systemtime, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons)
{
    pixel_matrix_->SetColonsOff();
    pixel_matrix_->SetColon(systemtime[ltc::systemtime::index::COLON_1], 2, colours_colons.red, colours_colons.green, colours_colons.blue);
    pixel_matrix_->SetColon(systemtime[ltc::systemtime::index::COLON_2], 4, colours_colons.red, colours_colons.green, colours_colons.blue);

    const char kLine[] = {' ', systemtime[0], systemtime[1], systemtime[3], systemtime[4], systemtime[6], systemtime[7], ' '};

    pixel_matrix_->TextLine(1, kLine, sizeof(kLine), colours.red, colours.green, colours.blue);
    pixel_matrix_->Show();
}

void LtcDisplayPixelMatrix::ShowMessage(const char* message, struct ltc::display::rgb::Colours& colours)
{
    pixel_matrix_->SetColonsOff();
    pixel_matrix_->TextLine(1, message, ltc::display::rgb::kMaxMessageSize, colours.red, colours.green, colours.blue);
    pixel_matrix_->Show();
}

void LtcDisplayPixelMatrix::WriteChar(uint8_t ch, uint8_t pos, struct ltc::display::rgb::Colours& colours)
{
    pixel_matrix_->SetCursorPos(pos, 0);
    pixel_matrix_->PutChar(static_cast<char>(ch), colours.red, colours.green, colours.blue);
    pixel_matrix_->Show();
}

void LtcDisplayPixelMatrix::Print()
{
    printf(" Matrix %dx%d\n", pixel_matrix_->GetMaxPosition(), pixel_matrix_->GetMaxLine());
}
