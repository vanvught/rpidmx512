/**
 * @file tlc59711.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef TLC59711_H_
#define TLC59711_H_

#include <cstdint>
#include <cassert>
#include <cstring>

namespace tlc59711
{
enum class Type : uint8_t
{
    kRgb,
    kRgbw,
    kUndefined
};

inline constexpr auto kTypesMaxNameLength = 10U;
inline constexpr const char kTypes[static_cast<uint32_t>(tlc59711::Type::kUndefined)][kTypesMaxNameLength] = {
    "TLC59711", // RGB led's
    "TLC59711W"   // RGBW led's
};

[[nodiscard]] inline constexpr const char* GetType(tlc59711::Type type)
{
    return type < tlc59711::Type::kUndefined ? kTypes[static_cast<uint32_t>(type)] : "Undefined";
}

inline tlc59711::Type GetType(const char* value)
{
    assert(value != nullptr);
    if (strcasecmp(value, kTypes[static_cast<uint32_t>(tlc59711::Type::kRgbw)]) == 0)
    {
        return tlc59711::Type::kRgbw;
    }

    return tlc59711::Type::kRgb;
}
} // namespace tlc59711

struct TLC59711SpiSpeed
{
    static constexpr uint32_t DEFAULT = 5000000; // 5 MHz
    static constexpr uint32_t MAX = 10000000;    // 10 MHz
};

struct TLC59711Channels
{
    static constexpr uint32_t U16BIT = 14;
    static constexpr uint32_t OUT = 12;
    static constexpr uint32_t RGB = 4;
};

class TLC59711
{
   public:
    explicit TLC59711(uint8_t boards = 1, uint32_t spi_speed_hz = TLC59711SpiSpeed::DEFAULT);
    ~TLC59711();

    int GetBlank() const;
    void SetBlank(bool blank = false);

    int GetDisplayRepeat() const;
    void SetDisplayRepeat(bool display_repeat = true);

    int GetDisplayTimingReset() const;
    void SetDisplayTimingReset(bool display_timing_reset = true);

    int GetExternalClock() const;
    void SetExternalClock(bool external_clock = false);

    int GetOnOffTiming() const;
    void SetOnOffTiming(bool on_off_timing = false);

    uint8_t GetGbcRed() const;
    void SetGbcRed(uint8_t value = 0x7F);

    uint8_t GetGbcGreen() const;
    void SetGbcGreen(uint8_t value = 0x7F);

    uint8_t GetGbcBlue() const;
    void SetGbcBlue(uint8_t value = 0x7F);

    bool Get(uint32_t channel, uint16_t& value);
    void Set(uint32_t channel, uint16_t value);

    void Set(uint32_t channel, uint8_t value);

    bool GetRgb(uint8_t out, uint16_t& red, uint16_t& green, uint16_t& blue);
    void SetRgb(uint8_t out, uint16_t red, uint16_t green, uint16_t blue);

    void SetRgb(uint8_t out, uint8_t red, uint8_t green, uint8_t blue);

    void Update();
    void Blackout();

    void Dump();

   private:
    void UpdateFirst32();

   private:
    uint8_t boards_;
    uint32_t spi_speed_hz_;
    uint32_t first32_{0};
    uint16_t* buffer_{nullptr};
    uint16_t* buffer_blackout_{nullptr};
    uint32_t buffer_size_{0};
};

#endif  // TLC59711_H_
