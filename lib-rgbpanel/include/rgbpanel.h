/**
 * @file rgbpanel.h
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

#ifndef RGBPANEL_H_
#define RGBPANEL_H_

#include <cstdint>
#include <cstring>
#include <cassert>

namespace rgbpanel
{
inline constexpr uint32_t kPwmWidth = 84;

enum class Types : uint8_t
{
    kHuB75,
    kFM6126A,
    kFM6127,
    kUndefined
};

inline constexpr uint32_t kTypeMaxNameLength = 7 + 1; // + '\0'
inline constexpr const char kTypes[static_cast<unsigned>(Types::kUndefined)][kTypeMaxNameLength] = {"HUB75", "FM6126A", "FM6127"};

inline const char* GetType(Types type)
{
    if (type < Types::kUndefined)
    {
        return kTypes[static_cast<uint32_t>(type)];
    }

    return "UNDEFINED";
}

inline Types GetType(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&type)[kTypeMaxNameLength] : kTypes)
    {
        if (strcasecmp(string, type) == 0)
        {
            return static_cast<Types>(index);
        }
        ++index;
    }

    return Types::kUndefined;
}

namespace defaults
{
inline constexpr uint32_t kCols = 32;
inline constexpr uint32_t kRows = 32;
inline constexpr uint32_t kChain = 1;
inline constexpr auto kType = Types::kHuB75;
} // namespace defaults
namespace config
{
inline constexpr uint32_t kCols = 2;
inline constexpr uint32_t kRows = 4;
} // namespace config

inline constexpr uint32_t kCols[config::kCols] = {32, 64};
inline constexpr uint32_t kRows[config::kRows] = {8, 16, 32, 64};

inline uint32_t ValidateColumns(uint32_t columns)
{
    for (uint32_t i = 0; i < config::kCols; i++)
    {
        if (columns == kCols[i])
        {
            return columns;
        }
    }

    return defaults::kCols;
}

inline uint32_t ValidateRows(uint32_t rows)
{
    for (uint32_t i = 0; i < config::kRows; i++)
    {
        if (rows == kRows[i])
        {
            return rows;
        }
    }

    return defaults::kRows;
}

} // namespace rgbpanel

class RgbPanel
{
   public:
    RgbPanel(uint32_t columns, uint32_t rows, uint32_t chain = rgbpanel::defaults::kChain, rgbpanel::Types type = rgbpanel::defaults::kType);

    RgbPanel(const RgbPanel&) = delete;
    RgbPanel& operator=(const RgbPanel&) = delete;
    RgbPanel(RgbPanel&&) = delete;
    RgbPanel& operator=(RgbPanel&&) = delete;

    ~RgbPanel() { PlatformCleanUp(); }

    void Start();
    void Stop();

    void SetPixel(uint32_t column, uint32_t row, uint8_t red, uint8_t green, uint8_t blue);
    void Cls();
    void Show();

    uint32_t GetShowCounter();
    uint32_t GetUpdatesCounter();

    void Dump();

    void PutChar(char ch, uint8_t red, uint8_t green, uint8_t blue);
    void PutString(const char* string, uint8_t red, uint8_t green, uint8_t blue);
    void Text(const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue);
    void TextLine(uint8_t line, const char* text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue);
    void SetCursorPos(uint8_t col, uint8_t row);
    void ClearLine(uint8_t line);
    void SetColon(char ch, uint8_t col, uint8_t row, uint8_t red, uint8_t green, uint8_t blue);
    void SetColonsOff();

    uint32_t GetMaxPosition() { return max_position_; }

    uint32_t GetMaxLine() { return max_line_; }

    void Print();

    uint32_t GetColumns() const { return columns_; }
    uint32_t GetRows() const { return rows_; }
    uint32_t GetChain() const { return chain_; }
    rgbpanel::Types GetType() const { return type_; }

    static RgbPanel& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

   private:
    void PlatformInit();
    void PlatformCleanUp();

   protected:
    uint32_t columns_;
    uint32_t rows_;

   private:
    uint32_t chain_;
    rgbpanel::Types type_;
    bool started_{false};
    // Text
    uint32_t position_{0};
    uint32_t max_position_;
    uint32_t max_line_;
    uint32_t line_{0};
    struct TColon
    {
        uint8_t bits;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    TColon* colons_{nullptr};

    static inline RgbPanel* s_this;
};

#endif  // RGBPANEL_H_
