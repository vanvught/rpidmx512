/**
 * @file display.h
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_DISPLAY_H_
#define SPI_DISPLAY_H_

#if !defined(CONFIG_DISPLAY_USE_SPI)
#error
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if defined(CONFIG_SPI_LCD_OPTIMIZE_O2) || defined(CONFIG_SPI_LCD_OPTIMIZE_O3)
#pragma GCC push_options
#if defined(CONFIG_SPI_LCD_OPTIMIZE_O2)
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

#if defined(CONFIG_USE_ILI9341)
#include "spi/ili9341.h"
using LcdDriver = ILI9341;
#elif defined(CONFIG_USE_ST7735S)
#include "spi/st7735s.h"
using LcdDriver = ST7735S;
#else
#include "spi/st7789.h"
using LcdDriver = ST7789;
#endif
#include "spi/lcd_font.h"
#include "spi/spilcd.h"
#include "console.h"
#if defined(DISPLAYTIMEOUT_GPIO)
#include "hal_gpio.h"
#endif

#if defined(SPI_LCD_HAVE_CS_GPIO)
inline constexpr uint32_t CS_GPIO = SPI_LCD_CS_GPIO;
#else
inline constexpr uint32_t CS_GPIO = 0;
#endif

 #include "firmware/debug/debug_debug.h"

class Display : public LcdDriver
{
   public:
    Display(uint32_t cs = CS_GPIO) : LcdDriver(cs)
    {
        DEBUG_ENTRY();

        s_this = this;

        SetBackLight(1);
        SetFlipVertically(false);
        FillColour(kColorBackground);

        cols_ = (GetWidth() / s_pFONT->kWidth);
        rows_ = (GetHeight() / s_pFONT->kHeight);
#if defined(DISPLAYTIMEOUT_GPIO)
        FUNC_PREFIX(GpioFsel(DISPLAYTIMEOUT_GPIO, GPIO_FSEL_INPUT));
        FUNC_PREFIX(GpioSetPud(DISPLAYTIMEOUT_GPIO, GPIO_PULL_UP));
#endif

        PrintInfo();
        DEBUG_EXIT();
    }

    ~Display() {}

    bool isDetected() const { return true; }

    void PrintInfo()
    {
#if defined(CONFIG_USE_ILI9341)
        printf("ILI9341 ");
#elif defined(CONFIG_USE_ST7735S)
        printf("ST7735S ");
#else
        printf("ST7789 ");
#endif
        printf("(%u,%u)\n", rows_, cols_);
    }

    void Cls() { FillColour(kColorBackground); }

    void SetCursorPos(const uint32_t nCol, const uint32_t row)
    {
        cursor_x_ = nCol * s_pFONT->kWidth;
        cursor_y_ = row * s_pFONT->kHeight;
    }

    void PutChar(const int c)
    {
        DrawChar(cursor_x_, cursor_y_, static_cast<char>(c), s_pFONT, kColorBackground, kColorForeground);

        cursor_x_ += s_pFONT->kWidth;

        if (cursor_x_ >= GetWidth())
        {
            cursor_x_ = 0;

            cursor_y_ += s_pFONT->kHeight;

            if (cursor_y_ >= GetHeight())
            {
                cursor_y_ = 0;
            }
        }
    }

    void PutString(const char* p)
    {
        for (uint32_t i = 0; *p != '\0'; i++)
        {
            PutChar(static_cast<int>(*p));
            p++;
        }
    }

    void ClearLine(const uint32_t nLine)
    {
        if (__builtin_expect((!(nLine <= rows_)), 0))
        {
            return;
        }

        SetCursorPos(0, (nLine - 1U));

        for (uint32_t i = 0; i < cols_; i++)
        {
            PutChar(' ');
        }

        SetCursorPos(0, (nLine - 1U));
    }

    void TextLine(const uint32_t nLine, const char* pText, const uint32_t nLength)
    {
        if (__builtin_expect((!(nLine <= rows_)), 0))
        {
            return;
        }

        SetCursorPos(0, (nLine - 1U));
        Text(pText, nLength);
    }

    void ClearEndOfLine() { clear_end_of_line_ = true; }

    void Text(const char* pData, uint32_t nLength)
    {
        if (nLength > cols_)
        {
            nLength = cols_;
        }

        for (uint32_t i = 0; i < nLength; i++)
        {
            PutChar(pData[i]);
        }
    }

    int Write(const uint32_t nLine, const char* pText)
    {
        const auto* p = pText;
        int nCount = 0;

        const auto columns = static_cast<int>(cols_);

        while ((*p != 0) && (nCount++ < columns))
        {
            ++p;
        }

        TextLine(nLine, pText, static_cast<uint8_t>(nCount));

        return nCount;
    }

    int Printf(const uint8_t nLine, const char* format, ...)
    {
        char buffer[32];

        va_list arp;

        va_start(arp, format);

        auto i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

        va_end(arp);

        TextLine(nLine, buffer, static_cast<uint16_t>(i));

        return i;
    }

    void TextStatus(const char* pText)
    {
        SetCursorPos(0, static_cast<uint8_t>(rows_ - 1));

        for (uint32_t i = 0; i < (cols_ - 1); i++)
        {
            PutChar(' ');
        }

        SetCursorPos(0, static_cast<uint8_t>(rows_ - 1));

        Write(rows_, pText);
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

    void Progress()
    {
        static constexpr char SYMBOLS[] = {'/', '-', '\\', '|'};
        static uint32_t nSymbolsIndex;

        SetCursorPos(GetColumns() - 1U, GetRows() - 1U);
        PutChar(SYMBOLS[nSymbolsIndex++]);

        if (nSymbolsIndex >= sizeof(SYMBOLS))
        {
            nSymbolsIndex = 0;
        }
    }

    void SetContrast(const uint8_t nContrast) { SetBackLight(nContrast); }

    void SetSleep(const bool bSleep)
    {
        is_sleep_ = bSleep;

        EnableSleep(bSleep);

        if (!bSleep)
        {
            SetSleepTimer(sleep_timeout_ != 0);
        }
    }

    bool IsSleep() const { return is_sleep_; }

    void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::kSleepTimeout)
    {
        sleep_timeout_ = 1000U * 60U * nSleepTimeout;
        SetSleepTimer(sleep_timeout_ != 0);
    }

    uint32_t GetSleepTimeout() const { return sleep_timeout_ / 1000U / 60U; }

    void SetFlipVertically(bool doFlipVertically) { SetRotation(doFlipVertically ? 3 : 1); }

    uint32_t GetColumns() const { return cols_; }

    uint32_t GetRows() const { return rows_; }

    uint8_t GetContrast() const { return contrast_; }

    bool GetFlipVertically() const { return is_flipped_vertically_; }

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

    static Display* Get() { return s_this; }

   private:
    void SetSleepTimer(const bool bActive);

   private:
    uint32_t cols_;
    uint32_t rows_;
    uint32_t sleep_timeout_{1000U * 60U * display::Defaults::kSleepTimeout};
    uint32_t cursor_x_{0};
    uint32_t cursor_y_{0};

    uint8_t contrast_{0x7F};

    bool is_flipped_vertically_{false};
    bool is_sleep_{false};
    bool clear_end_of_line_{false};

    static inline Display* s_this;

#if defined(SPI_LCD_240X320)
    static constexpr sFONT* s_pFONT = &Font16x24;
#elif defined(SPI_LCD_128X128)
    static constexpr sFONT* s_pFONT = &Font8x8;
#elif defined(SPI_LCD_160X80)
    static constexpr sFONT* s_pFONT = &Font8x8;
#else
    static constexpr sFONT* s_pFONT = &Font12x12;
#endif
    static constexpr uint16_t kColorBackground = 0x001F;
    static constexpr uint16_t kColorForeground = 0xFFE0;
};

#if defined(__GNUC__) && !defined(__clang__)
#if defined(CONFIG_SPI_LCD_OPTIMIZE_O2) || defined(CONFIG_SPI_LCD_OPTIMIZE_O3)
#pragma GCC pop_options
#endif
#endif

#endif  // SPI_DISPLAY_H_
