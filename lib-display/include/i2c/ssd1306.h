/**
 * @file ssd1306.h
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

#ifndef I2C_SSD1306_H_
#define I2C_SSD1306_H_

#include <cstdint>

#include "displayset.h"
#include "hal_i2c.h"

#define OLED_I2C_ADDRESS_DEFAULT 0x3C

enum class OledPanel
{
    k128x648Rows, ///< Default
    k128x644Rows,
    k128x324Rows
};

class Ssd1306 final : public DisplaySet
{
   public:
    Ssd1306();
    explicit Ssd1306(OledPanel);
    Ssd1306(uint8_t, OledPanel);
    ~Ssd1306() override
    {
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
        delete[] shadow_ram_;
        shadow_ram_ = nullptr;
#endif
    }

    bool Start() override;

    void Cls() override;
    void ClearLine(uint32_t line) override;

    void PutChar(int) override;
    void PutString(const char*) override;

    void Text(const char* data, uint32_t length);
    void TextLine(uint32_t line, const char* data, uint32_t length) override;

    void SetCursorPos(uint32_t column, uint32_t row) override;
    void SetCursor(uint32_t) override;

    void SetSleep(bool sleep) override;
    void SetContrast(uint8_t contrast) override;

    void SetFlipVertically(bool do_flip_vertically) override;

    void PrintInfo() override;

    bool IsSH1106() { return have_sh1106_; }

    static Ssd1306* Get() { return s_this; }

   private:
    void CheckSH1106();
    void InitMembers();
    void SendCommand(uint8_t);
    void SendData(const uint8_t* data, uint32_t length);

    void SetCursorOn();
    void SetCursorOff();
    void SetCursorBlinkOn();
    void SetColumnRow(uint8_t column, uint8_t row);

    void DumpShadowRam();

   private:
    HAL_I2C hal_i2c_;
    OledPanel oled_panel_{OledPanel::k128x648Rows};
    bool have_sh1106_{false};
    uint32_t pages_;
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    char* shadow_ram_{nullptr};
    uint32_t shadow_ram_index_{0};
#endif
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    uint32_t cursor_mode_{display::cursor::kOff};
    uint8_t cursor_on_char_;
    uint8_t cursor_on_column_;
    uint8_t cursor_on_row_;
#endif

    static inline Ssd1306* s_this;
};

#endif  // I2C_SSD1306_H_
