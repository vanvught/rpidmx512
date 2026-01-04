/**
 * @file hd44780.h
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

#ifndef I2C_HD44780_H_
#define I2C_HD44780_H_

#include <cstdint>

#include "displayset.h"
#include "hal_i2c.h"

namespace hd44780::pcf8574t
{
inline constexpr uint8_t kDefaultAddress = 0x27;
inline constexpr uint8_t kTC2004Address = kDefaultAddress;
inline constexpr uint8_t kTC1602Address = 0x26;
} // namespace hd44780::pcf8574t

class Hd44780 final : public DisplaySet
{
   public:
    Hd44780();
    Hd44780(uint8_t cols, uint8_t rows);
    Hd44780(uint8_t slave_address, uint8_t cols, uint8_t rows);

    bool Start() override;

    void Cls() override;
    void ClearLine(uint32_t line) override;

    void PutChar(int) override;
    void PutString(const char*) override;

    void Text(const char* data, uint32_t length);
    void TextLine(uint32_t line, const char* data, uint32_t length) override;

    void SetCursorPos(uint32_t col, uint32_t row) override;
    void SetCursor(uint32_t) override;

    void PrintInfo() override;

   private:
    void Write4bits(uint8_t data);
    void WriteCmd(uint8_t cmd);
    void WriteReg(uint8_t reg);

   private:
    HAL_I2C hal_i2c_;
};

#endif  // I2C_HD44780_H_
