/**
 * @file st77xx.h
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

#ifndef SPI_ST77XX_H_
#define SPI_ST77XX_H_

#include <cstdint>

#include "spi/paint.h"
#include "spi/spilcd.h"
#include "hal_gpio.h"
 #include "firmware/debug/debug_debug.h"

namespace st77xx
{
namespace cmd
{
inline constexpr uint8_t kNop = 0x00;
inline constexpr uint8_t kSwreset = 0x01; ///< Software Reset
inline constexpr uint8_t kRddid = 0x04;   ///< Read Display ID
inline constexpr uint8_t kRddst = 0x09;   ///< Read Display Status
inline constexpr uint8_t kSlpin = 0x10;    ///< Sleep In
inline constexpr uint8_t kSlpout = 0x11;   ///< Sleep Out
inline constexpr uint8_t kPtlon = 0x12;    ///< Partial Display Mode On
inline constexpr uint8_t kNoron = 0x13;    ///< Normal Display Mode On
inline constexpr uint8_t kInvoff = 0x20;   ///< Display Inversion Off
inline constexpr uint8_t kInvon = 0x21;    ///< Display Inversion On
inline constexpr uint8_t kGamset = 0x26;   ///< Gamma Set
inline constexpr uint8_t kDispoff = 0x28;  ///< Display Offs
inline constexpr uint8_t kDispon = 0x29;   ///< Display On
inline constexpr uint8_t kCaSet = 0x2A;   ///< Column Address Set
inline constexpr uint8_t kRaset = 0x2B;    ///< Row Address Set
inline constexpr uint8_t kRamwr = 0x2C;    ///< Memory Write
inline constexpr uint8_t kRamrd = 0x2E;    ///< Memory Read
inline constexpr uint8_t kPtlar = 0x30;    ///< Partial Area
inline constexpr uint8_t kTeoff = 0x34;   ///< Tearing Effect Line OFF
inline constexpr uint8_t kTeon = 0x35;    ///< Tearing Effect Line ON .
inline constexpr uint8_t kMadctl = 0x36;  ///< Memory Data Access Control.
inline constexpr uint8_t kIdmoff = 0x38;  ///< Idle Mode Off .
inline constexpr uint8_t kColmod = 0x3A;  ///< Interface Pixel Format
} // namespace cmd
namespace data
{
/**
 * Memory Data Access Control Register (0x36H)
 * MAP:     D7  D6  D5  D4  D3  D2  D1  D0
 * param:   MY  MX  MV  ML  RGB MH  -   -
 *
 */
/* Page Address Order ('0': Top to Bottom, '1': the opposite) */
inline constexpr uint8_t kMadctlMy = 0x80;
/* Column Address Order ('0': Left to Right, '1': the opposite) */
inline constexpr uint8_t kMadctlMx = 0x40;
/* Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode) */
inline constexpr uint8_t kMadctlMv = 0x20;
/* Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite) */
inline constexpr uint8_t kMadctlMl = 0x10;
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
inline constexpr uint8_t kMadctlRgb = 0x00;
inline constexpr uint8_t kMadctlBgr = 0x08;
} // namespace data

namespace colour
{
inline constexpr uint16_t kBlack = 0x0000;
inline constexpr uint16_t kBlue = 0x001F;
inline constexpr uint16_t kCyan = 0x07FF;
inline constexpr uint16_t kDarkblue = 0X01CF;
inline constexpr uint16_t kGray = 0X8430;
inline constexpr uint16_t kGreen = 0x07E0;
inline constexpr uint16_t kMagenta = 0xF81F;
inline constexpr uint16_t kOrange = 0xFC00;
inline constexpr uint16_t kRed = 0xF800;
inline constexpr uint16_t kWhite = 0xFFFF;
inline constexpr uint16_t kYellow = 0xFFE0;
} // namespace colour

} // namespace st77xx

class ST77XX : public Paint
{
   public:
    explicit ST77XX(uint32_t cs) : Paint(cs)
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    ~ST77XX() override = default;

    void EnableDisplay(bool enable) { WriteCommand(enable ? st77xx::cmd::kDispon : st77xx::cmd::kDispoff); }

    void EnableSleep(bool enable) { WriteCommand(enable ? st77xx::cmd::kSlpin : st77xx::cmd::kSlpout); }

    void SetBackLight(uint32_t value) { FUNC_PREFIX(GpioWrite(SPI_LCD_BL_GPIO, value == 0 ? 0 : 1)); }

    void SetAddressWindow(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) override
    {
        const auto kStartX = x0 + shift_x_;
        const auto kEndX = x1 + shift_x_;
        const auto kStartY = y0 + shift_y_;
        const auto kEndY = y1 + shift_y_;

        WriteCommand(st77xx::cmd::kCaSet);
        {
            uint8_t data[] = {static_cast<uint8_t>(kStartX >> 8), static_cast<uint8_t>(kStartX), static_cast<uint8_t>(kEndX >> 8), static_cast<uint8_t>(kEndX)};
            WriteData(data, sizeof(data));
        }

        WriteCommand(st77xx::cmd::kRaset);
        {
            uint8_t data[] = {static_cast<uint8_t>(kStartY >> 8), static_cast<uint8_t>(kStartY), static_cast<uint8_t>(kEndY >> 8), static_cast<uint8_t>(kEndY)};
            WriteData(data, sizeof(data));
        }

        WriteCommand(st77xx::cmd::kRamwr);
    }

   protected:
    uint32_t shift_x_{0};
    uint32_t shift_y_{0};
};

#endif  // SPI_ST77XX_H_
