/**
 * @file display.h
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

#ifndef I2C_DISPLAY_H_
#define I2C_DISPLAY_H_

#if defined(CONFIG_DISPLAY_USE_SPI)
#error
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if defined(CONFIG_I2C_LCD_OPTIMIZE_O2) || defined(CONFIG_I2C_LCD_OPTIMIZE_O3)
#pragma GCC push_options
#if defined(CONFIG_I2C_LCD_OPTIMIZE_O2)
#pragma GCC optimize("O2")
#else
#pragma GCC optimize("O3")
#endif
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif
#endif

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "displayset.h"
#include "console.h"
#include "hal.h"
#if defined(DISPLAYTIMEOUT_GPIO)
#include "hal_gpio.h"
#endif

namespace display
{
enum class Type
{
    kPcf8574T1602,
    kPcf8574T2004,
    kSsd1306,
    kSsd1311,
    kUnknown
};
} // namespace display

class Display
{
   public:
    Display();
   
    explicit Display(uint32_t rows);
    explicit Display(display::Type type);
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
   
    ~Display()
    {
        s_this = nullptr;
        delete lcd_display_;
        lcd_display_ = nullptr;
    }

    bool IsDetected() const { return lcd_display_ == nullptr ? false : true; }

    display::Type GetDetectedType() const { return type_; }

    void PrintInfo()
    {
        if (lcd_display_ == nullptr)
        {
            puts("No display found");
            return;
        }

        lcd_display_->PrintInfo();
    }

    void Cls()
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->Cls();
    }

    void ClearLine(uint32_t line)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->ClearLine(line);
    }

    void PutChar(int c)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->PutChar(c);
    }

    void PutString(const char* text)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->PutString(text);
    }

    int Write(uint32_t line, const char* text)
    {
        if (lcd_display_ == nullptr)
        {
            return 0;
        }

        const auto* p = text;
        uint32_t count = 0;

        const auto kColumns = lcd_display_->GetColumns();

        while ((*p != 0) && (count++ < kColumns))
        {
            ++p;
        }

        lcd_display_->TextLine(line, text, count);

        return static_cast<int>(count);
    }

    int Printf(uint32_t line, const char* format, ...)
    {
        if (lcd_display_ == nullptr)
        {
            return 0;
        }

        char buffer[32];

        va_list arp;

        va_start(arp, format);

        auto i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

        va_end(arp);

        lcd_display_->TextLine(line, buffer, static_cast<uint32_t>(i));

        return i;
    }

    void TextLine(uint32_t line, const char* text, uint32_t length)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->TextLine(line, text, length);
    }

    void TextStatus(const char* text)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        const auto kColumns = lcd_display_->GetColumns();
        const auto kRows = lcd_display_->GetRows();

        assert(kColumns >= 1);
        assert(kRows >= 1);

        SetCursorPos(0, kRows - 1U);

        for (uint32_t i = 0; i < kColumns - 1U; i++)
        {
            PutChar(' ');
        }

        SetCursorPos(0, kRows - 1U);

        Write(kRows, text);
    }

    void TextStatus(const char* text, console::Colours colour)
    {
        TextStatus(text);

        if (static_cast<uint32_t>(colour) == UINT32_MAX)
        {
            return;
        }

        console::Status(colour, text);
    }

    void SetCursor(uint32_t mode)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->SetCursor(mode);
    }

    void SetCursorPos(uint32_t col, uint32_t row)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->SetCursorPos(col, row);
    }

    void SetContrast(uint8_t contrast)
    {
        contrast_ = contrast;

        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->SetContrast(contrast);
    }

    uint8_t GetContrast() const { return contrast_; }

    void SetFlipVertically(bool do_flip_vertically)
    {
        is_flipped_vertically_ = do_flip_vertically;

        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->SetFlipVertically(do_flip_vertically);
    }

    void ClearEndOfLine()
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        lcd_display_->ClearEndOfLine();
    }

    bool GetFlipVertically() const { return is_flipped_vertically_; }

    uint32_t GetColumns() const
    {
        if (lcd_display_ == nullptr)
        {
            return 0;
        }

        return lcd_display_->GetColumns();
    }

    uint32_t GetRows() const
    {
        if (lcd_display_ == nullptr)
        {
            return 0;
        }

        return lcd_display_->GetRows();
    }

    void Progress()
    {
        static constexpr char kSymbols[] = {'/', '-', '\\', '|'};
        static uint32_t s_symbols_index;

        SetCursorPos(GetColumns() - 1U, GetRows() - 1U);
        PutChar(kSymbols[s_symbols_index++]);

        if (s_symbols_index >= sizeof(kSymbols))
        {
            s_symbols_index = 0;
        }
    }

    void SetSleep(bool sleep)
    {
        if (lcd_display_ == nullptr)
        {
            return;
        }

        is_sleep_ = sleep;

        lcd_display_->SetSleep(sleep);

        if (!sleep)
        {
            SetSleepTimer(sleep_timeout_ != 0);
        }
    }

    bool IsSleep() const { return is_sleep_; }

    void SetSleepTimeout(uint32_t sleep_timeout = display::Defaults::kSleepTimeout)
    {
        sleep_timeout_ = 1000U * 60U * sleep_timeout;
        SetSleepTimer(sleep_timeout_ != 0);
    }

    uint32_t GetSleepTimeout() const { return sleep_timeout_ / 1000U / 60U; }

    void Run()
    {
        if (sleep_timeout_ == 0)
        {
            return;
        }

        if (is_sleep_)
        {
#if defined(DISPLAYTIMEOUT_GPIO)
            if (__builtin_expect(((FUNC_PREFIX(GpioLev(DISPLAYTIMEOUT_GPIO)) == 0)), 0))
            {
                SetSleep(false);
            }
#endif
        }
    }

    static Display* Get()
    {
        assert(s_this != nullptr);
        return s_this;
    }

   private:
    void Detect(display::Type display_type);
    void Detect(uint32_t rows);
    void SetSleepTimer(bool active);

   private:
    display::Type type_{display::Type::kUnknown};
    uint32_t sleep_timeout_{1000 * 60 * display::Defaults::kSleepTimeout};
    uint8_t contrast_{0x7F};

    bool is_sleep_{false};
    bool is_flipped_vertically_{false};

    DisplaySet* lcd_display_{nullptr};
    static inline Display* s_this;
};

#if defined(__GNUC__) && !defined(__clang__)
#if defined(CONFIG_I2C_LCD_OPTIMIZE)
#pragma GCC pop_options
#endif
#endif

#endif  // I2C_DISPLAY_H_
