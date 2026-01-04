/**
 * @file ltcdisplayrgb.h
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYRGB_H_
#define LTCDISPLAYRGB_H_

#include <cstdint>

#include "ltcdisplayrgbset.h"
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
#include "pixeltype.h"
#endif
#include "softwaretimers.h"
#include "hal.h"

namespace ltc::display::rgb
{
enum class Type
{
    kWS28Xx,
    kRgbpanel
};

enum class WS28xxType
{
    SEGMENT,
    MATRIX
};

enum class ColonBlinkMode
{
    OFF,
    DOWN,
    UP
};

enum class ColourIndex
{
    TIME,
    COLON,
    MESSAGE,
    FPS,
    INFO,
    SOURCE,
    LAST
};

struct Defaults
{
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    static constexpr auto kLedType = pixel::Type::WS2812B;
#endif
    static constexpr uint32_t kColourTime = 0x00FF0000;
    static constexpr uint32_t COLOUR_COLON = 0x00FFFC00;
    static constexpr uint32_t COLOUR_MESSAGE = 0x00FFFFFF;
    static constexpr uint32_t COLOUR_FPS = 0x00FF0000;
    static constexpr uint32_t COLOUR_INFO = 0x00808080;
    static constexpr uint32_t COLOUR_SOURCE = 0x00707070;
    static constexpr auto COLON_BLINK_MODE = ColonBlinkMode::UP;
    static constexpr uint8_t MASTER = 0xFF;
    static constexpr uint8_t GLOBAL_BRIGHTNESS = 0xFF;
};
} // namespace ltc::display::rgb

class LtcDisplayRgb
{
    static constexpr uint32_t kMessageTimeMs = 3000;

   public:
    LtcDisplayRgb(ltc::display::rgb::Type type, ltc::display::rgb::WS28xxType ws28xx_type);
    ~LtcDisplayRgb();

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    void SetMapping(pixel::Map map) { pixel_map_ = map; }
#endif

    void SetMaster(uint8_t value) { master_ = value; }

    void SetColonBlinkMode(ltc::display::rgb::ColonBlinkMode colon_blink_mode) { colon_blink_mode_ = colon_blink_mode; }

    void SetColour(uint32_t rgb, ltc::display::rgb::ColourIndex colour_index)
    {
        if (colour_index >= ltc::display::rgb::ColourIndex::LAST)
        {
            return;
        }
        colour_[static_cast<uint32_t>(colour_index)] = rgb;
    }

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    void Init(pixel::Type type = pixel::Type::WS2812B);
#else
    void Init();
#endif
    void Print();

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    void Show(const char* timecode);
    void ShowSysTime(const char* systemtime);
    void ShowFPS(ltc::Type type);
    void ShowSource(ltc::Source source);
    void ShowInfo(const char* info);

    void WriteChar(uint8_t ch, uint8_t pos = 0);

    static LtcDisplayRgb* Get() { return s_this; }

    void SetMessage(const char* message, uint32_t size);

    void SetRGB(uint8_t red, uint8_t green, uint8_t blue, ltc::display::rgb::ColourIndex colour_index)
    {
        switch (colour_index)
        {
            case ltc::display::rgb::ColourIndex::TIME:
                colours_time_.red = red;
                colours_time_.green = green;
                colours_time_.blue = blue;
                break;
            case ltc::display::rgb::ColourIndex::COLON:
                colours_colons_.red = red;
                colours_colons_.green = green;
                colours_colons_.blue = blue;
                break;
            case ltc::display::rgb::ColourIndex::MESSAGE:
                colours_message_.red = red;
                colours_message_.green = green;
                colours_message_.blue = blue;
                break;
            case ltc::display::rgb::ColourIndex::FPS:
                colours_fps_.red = red;
                colours_fps_.green = green;
                colours_fps_.blue = blue;
                break;
            case ltc::display::rgb::ColourIndex::INFO:
                colours_info_.red = red;
                colours_info_.green = green;
                colours_info_.blue = blue;
                break;
            case ltc::display::rgb::ColourIndex::SOURCE:
                colours_source_.red = red;
                colours_source_.green = green;
                colours_source_.blue = blue;
                break;
            default:
                break;
        }
    }

   private:
    void SetRGB(uint32_t rgb, ltc::display::rgb::ColourIndex colour_index)
    {
        const auto kRed = static_cast<uint8_t>((rgb & 0xFF0000) >> 16);
        const auto kGreen = static_cast<uint8_t>((rgb & 0xFF00) >> 8);
        const auto kBlue = static_cast<uint8_t>(rgb & 0xFF);

        SetRGB(kRed, kGreen, kBlue, colour_index);
    }

    void SetRGB(const char* string)
    {
        if (!isdigit(string[0]))
        {
            return;
        }

        const auto kColourIndex = static_cast<ltc::display::rgb::ColourIndex>((string[0] - '0'));

        if (kColourIndex >= ltc::display::rgb::ColourIndex::LAST)
        {
            return;
        }

        const auto kRGB = HexadecimalToDecimal(string + 1);

        SetRGB(kRGB, kColourIndex);
    }

    uint32_t HexadecimalToDecimal(const char* hex_value, uint32_t length = 6)
    {
        auto* src = const_cast<char*>(hex_value);
        uint32_t decimal = 0;

        while (length-- > 0)
        {
            const auto kC = *src;

            if (isxdigit(kC) == 0)
            {
                break;
            }

            const auto kNibble = kC > '9' ? static_cast<uint8_t>((kC | 0x20) - 'a' + 10) : static_cast<uint8_t>(kC - '0');
            decimal = (decimal << 4) | kNibble;
            src++;
        }

        return decimal;
    }

    void ShowMessage();

    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param buffer Pointer to the packet buffer.
     * @param size Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    ltc::display::rgb::Type type_;
    ltc::display::rgb::WS28xxType ws28xx_type_;
    uint8_t intensity_{ltc::display::rgb::Defaults::GLOBAL_BRIGHTNESS};
    int32_t handle_{-1};
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    pixel::Map pixel_map_{pixel::Map::UNDEFINED};
    pixel::Type pixel_type_{pixel::Type::UNDEFINED};
#endif
    uint32_t colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::LAST)];
    uint32_t master_{ltc::display::rgb::Defaults::MASTER};
    uint32_t message_timer_{0};
    uint32_t colon_blink_millis_{0};
    char message_[ltc::display::rgb::kMaxMessageSize];
    char seconds_previous_{60};
    ltc::display::rgb::ColonBlinkMode colon_blink_mode_{ltc::display::rgb::Defaults::COLON_BLINK_MODE};

    LtcDisplayRgbSet* display_rgb_{nullptr};

    struct ltc::display::rgb::Colours colours_time_;
    struct ltc::display::rgb::Colours colours_colons_;
    struct ltc::display::rgb::Colours colours_message_;
    struct ltc::display::rgb::Colours colours_fps_;
    struct ltc::display::rgb::Colours colours_info_;
    struct ltc::display::rgb::Colours colours_source_;

    static inline LtcDisplayRgb* s_this;
};

#endif  // LTCDISPLAYRGB_H_
