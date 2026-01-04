/**
 * @file st7735s.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * ST7735S
 * 132RGB x 162dot 262K Color with Frame Memory
 * Single-Chip TFT Controller/Driver
 * Datasheet
 * Version 1.1
 * 2011/11
 */

#ifndef SPI_ST7735S_H_
#define SPI_ST7735S_H_

#include <cstdint>
#include <cassert>

#include "spi/config.h"
#include "spi/st77xx.h"
 #include "firmware/debug/debug_debug.h"

namespace st7735s
{
namespace cmd
{
inline constexpr uint8_t kInvctr = 0xB4;  ///< Display Inversion Control
inline constexpr uint8_t kPwctR1 = 0xC0;  ///< Power Control 1
inline constexpr uint8_t kPwctR2 = 0xC1;  ///< Power Control 2
inline constexpr uint8_t kPwctR3 = 0xC2;  ///< Power Control 3 in normal mode, full colors
inline constexpr uint8_t kPwctR4 = 0xC3;  ///< Power Control 4 in idle mode 8colors
inline constexpr uint8_t kPwctR5 = 0xC4;  ///< Power Control 5 in partial mode, full colors
inline constexpr uint8_t kGmctrP1 = 0xE0; ///< Gamma (‘+’polarity) Correction Characteristics Setting
inline constexpr uint8_t kGmctrN1 = 0xE1; ///< Gamma ‘-’polarity Correction Characteristics Setting
} // namespace cmd
#if defined(SPI_LCD_128X128)
inline constexpr uint32_t kRotation0ShiftX = 0;
inline constexpr uint32_t kRotation0ShiftY = 0;
inline constexpr uint32_t kRotation1ShiftX = 3;
inline constexpr uint32_t kRotation1ShiftY = 2;
inline constexpr uint32_t kRotation2ShiftX = 0;
inline constexpr uint32_t kRotation2ShiftY = 0;
inline constexpr uint32_t kRotation3ShiftX = 1;
inline constexpr uint32_t kRotation3ShiftY = 1;
#elif defined(SPI_LCD_160X80)
inline constexpr uint32_t kRotation0ShiftX = 0;
inline constexpr uint32_t kRotation0ShiftY = 0;
inline constexpr uint32_t kRotation1ShiftX = 0;
inline constexpr uint32_t kRotation1ShiftY = 24;
inline constexpr uint32_t kRotation2ShiftX = 0;
inline constexpr uint32_t kRotation2ShiftY = 0;
inline constexpr uint32_t kRotation3ShiftX = 0;
inline constexpr uint32_t kRotation3ShiftY = 24;
#endif
} // namespace st7735s

class ST7735S : public ST77XX
{
   public:
    explicit ST7735S(uint32_t cs) : ST77XX(cs)
    {
        DEBUG_ENTRY();

        if (s_instance == 0)
        {
            HardwareReset();
        }

        s_instance++;

        WriteCommand(st77xx::cmd::kSwreset);
        udelay(1000 * 150);

        static constexpr uint8_t kConfig[] = {
            1,  st77xx::cmd::kColmod,    0x05, ///< Page 150, 5 -> 16-bit/pixel
            1,  st77xx::cmd::kGamset,    0x08, ///< Page 125, 8 -> Gamma Curve 4
            1,  st7735s::cmd::kInvctr,  0x01, ///< Page 162, Display Inversion mode control -> 3-bit,  0=Dot, 1=Column
            16, st7735s::cmd::kGmctrP1, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2c, 0x29, 0x25, 0x2b, 0x39, 0x00, 0x01, 0x03, 0x10,
            16, st7735s::cmd::kGmctrN1, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2c, 0x2e, 0x2e, 0x37, 0x3f, 0x00, 0x00, 0x02, 0x10,
            0,  st77xx::cmd::kInvoff, ///< Page 124, Enter into display inversion mode
            0,  st77xx::cmd::kIdmoff, ///< Page 147, Recover from Idle mode on
            0,  st77xx::cmd::kNoron   ///< Page 122, Normal Display mode on
        };

        uint32_t arg_length = 0;

        for (uint32_t i = 0; i < sizeof(kConfig); i += (arg_length + 2))
        {
            arg_length = kConfig[i];
            DEBUG_PRINTF("i=%u, arg_length=%u", i, arg_length);
            WriteCommand(&kConfig[i + 1], arg_length);
        }

        SetRotation(0);

        WriteCommand(st77xx::cmd::kSlpout); ///< Sleep Out
        WriteCommand(st77xx::cmd::kDispon); ///< Display On

        DEBUG_EXIT();
    }

    ~ST7735S() override {};

    void SetRotation(uint32_t rotation)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("rotation=%u", rotation);

        WriteCommand(st77xx::cmd::kMadctl);

        switch (rotation)
        {
            case 0:
                WriteDataByte(st77xx::data::kMadctlMx | st77xx::data::kMadctlMy | st77xx::data::kMadctlBgr);
                shift_x_ = st7735s::kRotation0ShiftX;
                shift_y_ = st7735s::kRotation0ShiftY;
                width_ = config::kWidth;
                height_ = config::kHeight;
                break;
            case 1:
                WriteDataByte(st77xx::data::kMadctlMy | st77xx::data::kMadctlMv | st77xx::data::kMadctlBgr);
                shift_x_ = st7735s::kRotation1ShiftX;
                shift_y_ = st7735s::kRotation1ShiftY;
                width_ = config::kHeight;
                height_ = config::kWidth;
                break;
            case 2:
                WriteDataByte(st77xx::data::kMadctlBgr);
                shift_x_ = st7735s::kRotation2ShiftX;
                shift_y_ = st7735s::kRotation2ShiftY;
                width_ = config::kWidth;
                height_ = config::kHeight;
                break;
            case 3:
                WriteDataByte(st77xx::data::kMadctlMx | st77xx::data::kMadctlMv | st77xx::data::kMadctlBgr);
                shift_x_ = st7735s::kRotation3ShiftX;
                shift_y_ = st7735s::kRotation3ShiftY;
                width_ = config::kHeight;
                height_ = config::kWidth;
                break;
            default:
                assert(0);
                __builtin_unreachable();
                break;
        }

        rotate_ = rotation;

        DEBUG_EXIT();
    }

   private:
    static inline uint32_t s_instance;
};

#endif  // SPI_ST7735S_H_
