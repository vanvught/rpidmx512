/**
 * @file ssd1311.h
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

#ifndef I2C_SSD1311_H_
#define I2C_SSD1311_H_

#include <cstdint>

#include "displayset.h"
#include "hal_i2c.h"

class Ssd1311 final : public DisplaySet
{
   public:
    Ssd1311();
    ~Ssd1311() override = default;

    bool Start() override;

    void Cls() override;
    void ClearLine(uint32_t line) override;

    void PutChar(int) override;
    void PutString(const char*) override;

    void Text(const char* data, uint32_t length);
    void TextLine(uint32_t line, const char* data, uint32_t length) override;

    void SetCursorPos(uint32_t col, uint32_t row) override;
    void SetCursor(uint32_t) override;

    void SetSleep(bool sleep) override;
    void SetContrast(uint8_t contrast) override;

    void PrintInfo() override;

    static Ssd1311* Get() { return s_this; }

   private:
    bool CheckSSD1311();
    void SelectRamRom(uint32_t ram, uint32_t rom);
    void SetDDRAM(uint8_t address);
    void SetCGRAM(uint8_t address);
    enum class FunctionSet
    {
        kReZero = (0 << 1),
        kReOne = (1 << 1)
    };
    void SetRE(FunctionSet re);
    enum class CommandSet
    {
        kDisabled = 0,
        kEnabled = 1
    };
    void SetSD(CommandSet sd);
    void SendCommand(uint8_t command);
    void SendData(uint8_t data);
    void SendData(const uint8_t* data, uint32_t length);

   private:
    HAL_I2C hal_i2_c_;
    uint8_t display_control_{1U << 3}; // Section 9.1.4 Display ON/OFF Control

    static inline Ssd1311* s_this;
};

#endif  // I2C_SSD1311_H_
