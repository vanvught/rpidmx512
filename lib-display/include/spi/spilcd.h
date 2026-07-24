/**
 * @file spilcd.h
 *
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_SPILCD_H_
#define SPI_SPILCD_H_

#include "timing.h"
#include "spi/config/config_lcd.h"
#include "spi.h"
#include "gpio.h"
#include "display_debug.h"

class SpiLcd {
   public:
    explicit SpiLcd(uint32_t chip_select = 0) : cs_(chip_select) {
        DISPLAY_DEBUG_ENTRY();
        DISPLAY_DEBUG_PRINTF("chip_select=%u", static_cast<unsigned>(chip_select));

        spi::Begin();
        spi::ChipSelect(spi::kCsNone);
        spi::SetSpeedHz(20000000);
        spi::SetDataMode(spi::kMode0);

#if defined(SPI_LCD_RST_GPIO)
        gpio::Fsel(SPI_LCD_RST_GPIO, gpio::Select::kOutput);
#endif
        gpio::Fsel(SPI_LCD_DC_GPIO, gpio::Select::kOutput);
        gpio::Fsel(SPI_LCD_BL_GPIO, gpio::Select::kOutput);
#if defined(SPI_LCD_HAVE_CS_GPIO)
        gpio::Fsel(cs_, gpio::Select::kOutput);
#endif

        DISPLAY_DEBUG_EXIT();
    }

    void HardwareReset() {
#if defined(SPI_LCD_RST_GPIO)
        timing::DelayUs(1000 * 200);
        gpio::Clr(SPI_LCD_RST_GPIO);
        timing::DelayUs(1000 * 200);
        gpio::Set(SPI_LCD_RST_GPIO);
        timing::DelayUs(1000 * 200);
#endif
    }

    void SetCS() {
#if defined(SPI_LCD_HAVE_CS_GPIO)
        gpio::Set(cs_);
#endif
    }

    void ClearCS() {
#if defined(SPI_LCD_HAVE_CS_GPIO)
        gpio::Clr(cs_);
#endif
    }

    void SetDC() { gpio::Set(SPI_LCD_DC_GPIO); }

    void ClearDC() { gpio::Clr(SPI_LCD_DC_GPIO); }

    void WriteCommand(uint8_t data) {
        ClearCS();
        ClearDC();
        spi::Writenb(reinterpret_cast<char*>(&data), 1);
        SetCS();
    }

    void WriteData(const uint8_t* data, uint32_t length) {
        ClearCS();
        SetDC();
        spi::Writenb(reinterpret_cast<const char*>(data), length);
        SetCS();
    }

    void WriteCommand(const uint8_t* data, uint32_t length) {
        auto* p = data;
        WriteCommand(p++[0]);
        if (length != 0) {
            WriteData(p, length);
        }
    }

    void WriteDataByte(uint8_t data) {
        ClearCS();
        SetDC();
        spi::Writenb(reinterpret_cast<char*>(&data), 1);
        SetCS();
    }

    void WriteDataWord(uint16_t data) {
        ClearCS();
        SetDC();
        spi::Write(data);
        SetCS();
    }

    void WriteDataStart(uint8_t* data, uint32_t length) {
        ClearCS();
        SetDC();
        spi::Writenb(reinterpret_cast<char*>(data), length);
    }

    void WriteDataContinue(uint8_t* data, uint32_t length) { spi::Writenb(reinterpret_cast<char*>(data), length); }

    void WriteDataEnd(uint8_t* data, uint32_t length) {
        spi::Writenb(reinterpret_cast<char*>(data), length);
        SetCS();
    }

   private:
    uint32_t cs_;
};

#endif // SPI_SPILCD_H_
