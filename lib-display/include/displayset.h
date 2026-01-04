/**
 * @file displayset.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DISPLAYSET_H_
#define DISPLAYSET_H_

#include <cstdint>

namespace display::cursor
{
inline constexpr uint32_t kOff = 0;
inline constexpr uint32_t kOn = (1U << 0);
inline constexpr uint32_t kBlinkOff = 0;
inline constexpr uint32_t kBlinkOn = (1U << 1);
} // namespace display::cursor

class DisplaySet
{
   public:
    virtual ~DisplaySet() = default;

    uint32_t GetColumns() const { return cols_; }

    uint32_t GetRows() const { return rows_; }

    void ClearEndOfLine() { clear_end_of_line_ = true; }

    virtual bool Start() = 0;

    virtual void Cls() = 0;
    virtual void ClearLine(uint32_t line) = 0;

    virtual void PutChar(int) = 0;
    virtual void PutString(const char*) = 0;

    virtual void TextLine(uint32_t line, const char* data, uint32_t length) = 0;

    virtual void SetCursorPos(uint32_t col, uint32_t row) = 0;
    virtual void SetCursor(uint32_t) = 0;

    virtual void SetSleep([[maybe_unused]] bool sleep) {}
    virtual void SetContrast([[maybe_unused]] uint8_t contrast) {}
    virtual void SetFlipVertically([[maybe_unused]] bool do_flip_vertically) {}

    virtual void PrintInfo() {}

   protected:
    uint32_t cols_;
    uint32_t rows_;
    bool clear_end_of_line_{false};
};

#endif  // DISPLAYSET_H_
