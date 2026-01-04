/**
 * @file st7789.h
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

#ifndef SPI_ST7789_H_
#define SPI_ST7789_H_

#include <cstdint>
#include <cassert>

#include "spi/config.h"
#include "spi/st77xx.h"
 #include "firmware/debug/debug_debug.h"

namespace st7789
{
namespace cmd
{
inline constexpr uint8_t kGctrl = 0xB7;    ///< Gate Control
inline constexpr uint8_t kVcoms = 0xBB;    ///< VCOM Setting
inline constexpr uint8_t kLcmctrl = 0xC0;  ///< LCM Control
inline constexpr uint8_t kVdvvrhen = 0xC2; ///< VDV and VRH Command Enable
inline constexpr uint8_t kVrhs = 0xC3;     ///< VRH Set
inline constexpr uint8_t kVdvs = 0xC4;     ///< VDV Set
inline constexpr uint8_t kFrctrL2 = 0xC6;  ///< Frame Rate Control in Normal Mode
inline constexpr uint8_t kPwctrL1 = 0xD0;  ///< Power Control 1
} // namespace cmd
#if defined(SPI_LCD_240X240)
inline constexpr uint32_t kRotation0ShiftX = 0;
inline constexpr uint32_t kRotation0ShiftY = 80;
inline constexpr uint32_t kRotation1ShiftX = 80;
inline constexpr uint32_t kRotation1ShiftY = 0;
inline constexpr uint32_t kRotation2ShiftX = 0;
inline constexpr uint32_t kRotation2ShiftY = 0;
inline constexpr uint32_t kRotation3ShiftX = 0;
inline constexpr uint32_t kRotation3ShiftY = 0;
#elif defined(SPI_LCD_240X320)
inline constexpr uint32_t kRotation0ShiftX = 0;
inline constexpr uint32_t kRotation0ShiftY = 0;
inline constexpr uint32_t kRotation1ShiftX = 0;
inline constexpr uint32_t kRotation1ShiftY = 0;
inline constexpr uint32_t kRotation2ShiftX = 0;
inline constexpr uint32_t kRotation2ShiftY = 0;
inline constexpr uint32_t kRotation3ShiftX = 0;
inline constexpr uint32_t kRotation3ShiftY = 0;
#endif
} // namespace st7789

class ST7789 : public ST77XX
{
   public:
    explicit ST7789(uint32_t cs) : ST77XX(cs)
    {
        DEBUG_ENTRY();

#if defined(SPI_LCD_RST_GPIO)
        if (s_instance == 0)
        {
            HardwareReset();
        }
        s_instance++;
#endif

        WriteCommand(st77xx::cmd::kSwreset);
        udelay(1000 * 150);

        static constexpr uint8_t kConfig[] = {1,
                                              st77xx::cmd::kColmod,
                                              0x55,
                                              1,
                                              st7789::cmd::kGctrl,
                                              0x35,
                                              1,
                                              st7789::cmd::kVcoms,
                                              0x19,
                                              1,
                                              st7789::cmd::kLcmctrl,
                                              0x2C,
                                              1,
                                              st7789::cmd::kVdvvrhen,
                                              0x01,
                                              1,
                                              st7789::cmd::kVrhs,
                                              0x12,
                                              1,
                                              st7789::cmd::kVdvs,
                                              0x20,
                                              1,
                                              st7789::cmd::kFrctrL2,
                                              0x0F,
                                              2,
                                              st7789::cmd::kPwctrL1,
                                              0xA4,
                                              0xA1,
                                              0,
                                              st77xx::cmd::kNoron,
                                              0,
                                              st77xx::cmd::kInvon};

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

    ~ST7789() override { DEBUG_ENTRY(); DEBUG_EXIT(); };

    void SetRotation(uint32_t rotation)
    {
        WriteCommand(st77xx::cmd::kMadctl);

        switch (rotation)
        {
            case 0:
                WriteDataByte(st77xx::data::kMadctlMx | st77xx::data::kMadctlMy | st77xx::data::kMadctlRgb);
                shift_x_ = st7789::kRotation0ShiftX;
                shift_y_ = st7789::kRotation0ShiftY;
                width_ = config::kWidth;
                height_ = config::kHeight;
                break;
            case 1:
                WriteDataByte(st77xx::data::kMadctlMy | st77xx::data::kMadctlMv | st77xx::data::kMadctlRgb);
                shift_x_ = st7789::kRotation1ShiftX;
                shift_y_ = st7789::kRotation1ShiftY;
                width_ = config::kHeight;
                height_ = config::kWidth;
                break;
            case 2:
                WriteDataByte(st77xx::data::kMadctlRgb);
                shift_x_ = st7789::kRotation2ShiftX;
                shift_y_ = st7789::kRotation2ShiftY;
                width_ = config::kWidth;
                height_ = config::kHeight;
                break;
            case 3:
                WriteDataByte(st77xx::data::kMadctlMx | st77xx::data::kMadctlMv | st77xx::data::kMadctlRgb);
                shift_x_ = st7789::kRotation3ShiftX;
                shift_y_ = st7789::kRotation3ShiftY;
                width_ = config::kHeight;
                height_ = config::kWidth;
                break;
            default:
                assert(0);
                __builtin_unreachable();
                break;
        }

        rotate_ = rotation;
    }

   private:
#if defined(SPI_LCD_RST_GPIO)
    static inline uint32_t s_instance;
#endif
};

#endif  // SPI_ST7789_H_
