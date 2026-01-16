/**
 * @file pixelconfiguration.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELCONFIGURATION_H_
#define PIXELCONFIGURATION_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "pixeltype.h"

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
#include "gamma/gamma_tables.h"
#endif

 #include "firmware/debug/debug_debug.h"

class PixelConfiguration
{
   public:
    PixelConfiguration()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        DEBUG_EXIT();
    }

    ~PixelConfiguration() = default;

    PixelConfiguration(const PixelConfiguration&) = delete;
    PixelConfiguration& operator=(const PixelConfiguration&) = delete;

    void SetType(pixel::Type type)
    {
        type_ = type;
        refresh_needed_ = true;
    }

    pixel::Type GetType() const { return type_; }

    void SetCount(uint32_t count)
    {
        count_ = (count == 0 ? pixel::defaults::kCount : count);
        refresh_needed_ = true;
    }

    uint32_t GetCount() const { return count_; }

    void SetMap(pixel::Map map) { map_ = map; }

    pixel::Map GetMap() const { return map_; }

    void SetLowCode(uint8_t low_code)
    {
        low_code_ = low_code;
        refresh_needed_ = true;
    }

    uint8_t GetLowCode() const { return low_code_; }

    void SetHighCode(uint8_t high_code)
    {
        high_code_ = high_code;
        refresh_needed_ = true;
    }

    uint8_t GetHighCode() const { return high_code_; }

    void SetClockSpeedHz(uint32_t clock_speed_hz)
    {
        clock_speed_hz_ = clock_speed_hz;
        refresh_needed_ = true;
    }

    uint32_t GetClockSpeedHz() const { return clock_speed_hz_; }

    void SetGlobalBrightness(uint8_t global_brightness) { global_brightness_ = global_brightness; }

    uint8_t GetGlobalBrightness() const { return global_brightness_; }

    bool IsRTZProtocol() const { return is_rtz_protocol_; }

    uint32_t GetLedsPerPixel() const { return leds_per_pixel_; }

    uint32_t GetRefreshRate() const { return refresh_rate_; }

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    void SetEnableGammaCorrection(bool do_enable) { enable_gamma_correction_ = do_enable; }

    bool IsEnableGammaCorrection() const { return enable_gamma_correction_; }

    void SetGammaTable(uint32_t value) { gamma_value_ = static_cast<uint8_t>(gamma::GetValidValue(value)); }
    uint8_t GetGammaTableValue() const { return gamma_value_; }

    const uint8_t* GetGammaTable() const { return gamma_table_; }
#endif

    void GetTxH(pixel::Type type, uint8_t& low_code, uint8_t& high_code)
    {
        low_code = 0xC0;
        high_code = (type == pixel::Type::WS2812B
                         ? 0xF8
                         : (((type == pixel::Type::UCS1903) || (type == pixel::Type::UCS2903) || (type == pixel::Type::CS8812)) ? 0xFC : 0xF0));
    }

    void Validate()
    {
        DEBUG_ENTRY();

        if (type_ == pixel::Type::SK6812W)
        {
            count_ = count_ <= pixel::max::ledcount::RGBW ? count_ : pixel::max::ledcount::RGBW;
            leds_per_pixel_ = 4;
        }
        else
        {
            count_ = count_ <= pixel::max::ledcount::RGB ? count_ : pixel::max::ledcount::RGB;
            leds_per_pixel_ = 3;
        }

        if ((type_ == pixel::Type::APA102) || (type_ == pixel::Type::SK9822))
        {
            if (global_brightness_ > 0x1F)
            {
                global_brightness_ = 0xFF;
            }
            else
            {
                global_brightness_ = 0xE0 | (global_brightness_ & 0x1F);
            }
        }

        if ((type_ == pixel::Type::WS2801) || (type_ == pixel::Type::APA102) || (type_ == pixel::Type::SK9822) || (type_ == pixel::Type::P9813))
        {
            is_rtz_protocol_ = false;

            if (map_ == pixel::Map::UNDEFINED)
            {
                map_ = pixel::Map::RGB;
            }

            if (type_ == pixel::Type::P9813)
            {
                if (clock_speed_hz_ == 0)
                {
                    clock_speed_hz_ = pixel::spi::speed::p9813::kDefaultHz;
                }
                else if (clock_speed_hz_ > pixel::spi::speed::p9813::kMaxHz)
                {
                    clock_speed_hz_ = pixel::spi::speed::p9813::kMaxHz;
                }
            }
            else
            {
                if (clock_speed_hz_ == 0)
                {
                    clock_speed_hz_ = pixel::spi::speed::ws2801::kDefaultHz;
                }
                else if (clock_speed_hz_ > pixel::spi::speed::ws2801::kMaxHz)
                {
                    clock_speed_hz_ = pixel::spi::speed::ws2801::kMaxHz;
                }
            }

            const auto kLedTime = (8U * 1000000U) / clock_speed_hz_;
            const auto kLedsTime = kLedTime * count_ * leds_per_pixel_;
            if (kLedsTime > 0)
            {
                refresh_rate_ = 1000000U / kLedsTime;
            }
            else
            {
                refresh_rate_ = 0;
                assert(0);
            }
        }
        else
        {
            is_rtz_protocol_ = true;

            if (type_ == pixel::Type::UNDEFINED)
            {
                type_ = pixel::Type::WS2812B;
            }

            if (map_ == pixel::Map::UNDEFINED)
            {
                map_ = pixel::GetMap(type_);
            }

            if (low_code_ >= high_code_)
            {
                low_code_ = 0;
                high_code_ = 0;
            }

            uint8_t low_code, high_code;

            GetTxH(type_, low_code, high_code);

            if (low_code_ == 0)
            {
                low_code_ = low_code;
            }

            if (high_code_ == 0)
            {
                high_code_ = high_code;
            }

            clock_speed_hz_ = 6400000; // 6.4MHz / 8 bits = 800Hz

            //                  8 * 1000.000
            // led time (us) =  ------------ * 8 = 10 us
            //                   6.400.000
            const auto kLedsTime = 10U * count_ * leds_per_pixel_;
            refresh_rate_ = 1000000U / kLedsTime;
        }

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
        if (enable_gamma_correction_)
        {
            if (gamma_value_ == 0)
            {
                gamma_table_ = gamma::GetTableDefault(type_);
            }
            else
            {
                gamma_table_ = gamma::GetTable(gamma_value_);
            }
        }
        else
        {
            gamma_table_ = gamma10_0;
        }

        gamma_value_ = gamma::GetValue(gamma_table_);
#endif

        DEBUG_EXIT();
    }

    bool RefreshNeeded() const { return refresh_needed_; }

    void RefreshNeededReset() { refresh_needed_ = false; }

    void Print()
    {
        puts("Pixel configuration");
        printf(" Type    : %s [%d] <%d leds/pixel>\n", pixel::GetType(type_), static_cast<int>(type_), static_cast<int>(leds_per_pixel_));
        printf(" Count   : %d\n", count_);

        if (is_rtz_protocol_)
        {
            printf(" Mapping : %s [%d]\n", pixel::GetMap(map_), static_cast<int>(map_));
            printf(" T0H     : %.2f [0x%X]\n", pixel::ConvertTxH(low_code_), low_code_);
            printf(" T1H     : %.2f [0x%X]\n", pixel::ConvertTxH(high_code_), high_code_);
        }
        else
        {
            if ((type_ == pixel::Type::APA102) || (type_ == pixel::Type::SK9822))
            {
                printf(" GlobalBrightness: %u\n", global_brightness_);
            }
        }

        printf(" Clock   : %u Hz\n", static_cast<unsigned int>(clock_speed_hz_));
        printf(" Refresh : %u Hz\n", static_cast<unsigned int>(refresh_rate_));

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
        printf(" Gamma correction %s\n", enable_gamma_correction_ ? "Yes" : "No");
        if (enable_gamma_correction_)
        {
            printf("   Value = %u\n", gamma_value_);
        }
#endif
    }

    static PixelConfiguration& Get()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

   private:
    uint32_t count_{pixel::defaults::kCount};
    uint32_t clock_speed_hz_{0};
    uint32_t leds_per_pixel_{3};
    pixel::Type type_{pixel::defaults::kType};
    pixel::Map map_{pixel::Map::UNDEFINED};
    bool is_rtz_protocol_{true};
    uint8_t low_code_{0};
    uint8_t high_code_{0};
    uint8_t global_brightness_{0xFF};
    uint32_t refresh_rate_{0};
    bool refresh_needed_{true};
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    uint8_t gamma_value_{0};
    bool enable_gamma_correction_{false};
    const uint8_t* gamma_table_{gamma10_0};
#endif

    static inline PixelConfiguration* s_this{nullptr};
};

#endif  // PIXELCONFIGURATION_H_
