/**
 * @file display.cpp
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

#if defined(DEBUG_DISPLAY)
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "display.h"
#include "displayset.h"
#include "i2c/ssd1306.h"
#if defined(CONFIG_DISPLAY_ENABLE_SSD1311)
#include "i2c/ssd1311.h"
#endif
#if defined(CONFIG_DISPLAY_ENABLE_HD44780)
#include "i2c/hd44780.h"
#endif
#include "hal_i2c.h"
#include "hal_gpio.h"

namespace display::timeout
{
void irq_init();
static void GpioInit()
{
#if defined(DISPLAYTIMEOUT_GPIO)
    FUNC_PREFIX(GpioFsel(DISPLAYTIMEOUT_GPIO, GPIO_FSEL_INPUT));
    FUNC_PREFIX(GpioSetPud(DISPLAYTIMEOUT_GPIO, GPIO_PULL_UP));
    irq_init();
#endif
}
} // namespace display::timeout

Display::Display()
{
    assert(s_this == nullptr);
    s_this = this;

#if defined(CONFIG_DISPLAY_ENABLE_SSD1311)
    Detect(display::Type::kSsd1311);
#endif

    if (lcd_display_ == nullptr)
    {
        Detect(display::Type::kSsd1306);
    }

    if (lcd_display_ != nullptr)
    {
        display::timeout::GpioInit();
    }

    PrintInfo();
}

Display::Display(uint32_t rows)
{
    assert(s_this == nullptr);
    s_this = this;

    Detect(rows);

    if (lcd_display_ != nullptr)
    {
        display::timeout::GpioInit();
    }

    PrintInfo();
}

Display::Display(display::Type type) : type_(type)
{
    assert(s_this == nullptr);
    s_this = this;

    Detect(type);

    if (lcd_display_ != nullptr)
    {
        display::timeout::GpioInit();
    }

    PrintInfo();
}

void Display::Detect(display::Type display_type)
{
    switch (display_type)
    {
#if defined(CONFIG_DISPLAY_ENABLE_HD44780)
        case display::Type::kPcf8574T1602:
            lcd_display_ = new Hd44780(16, 2);
            assert(lcd_display_ != nullptr);
            break;
        case display::Type::kPcf8574T2004:
            lcd_display_ = new Hd44780(20, 4);
            assert(lcd_display_ != nullptr);
            break;
#endif
#if defined(CONFIG_DISPLAY_ENABLE_SSD1311)
        case display::Type::kSsd1311:
            lcd_display_ = new Ssd1311;
            assert(lcd_display_ != nullptr);
            break;
#endif
        case display::Type::kSsd1306:
            lcd_display_ = new Ssd1306(OledPanel::k128x648Rows);
            assert(lcd_display_ != nullptr);
            break;
        case display::Type::kUnknown:
            type_ = display::Type::kUnknown;
            /* no break */
        default:
            break;
    }

    if (lcd_display_ != nullptr)
    {
        if (!lcd_display_->Start())
        {
            delete lcd_display_;
            lcd_display_ = nullptr;
            type_ = display::Type::kUnknown;
        }
        else
        {
            lcd_display_->Cls();
        }
    }

    if (lcd_display_ == nullptr)
    {
        sleep_timeout_ = 0;
    }
}

void Display::Detect(uint32_t rows)
{
    if (HAL_I2C::IsConnected(OLED_I2C_ADDRESS_DEFAULT))
    {
        if (rows <= 4)
        {
#if defined(CONFIG_DISPLAY_ENABLE_SSD1311)
            lcd_display_ = new Ssd1311;
            assert(lcd_display_ != nullptr);

            if (lcd_display_->Start())
            {
                type_ = display::Type::kSsd1311;
                Printf(1, "SSD1311");
            }
            else
#endif
            {
                lcd_display_ = new Ssd1306(OledPanel::k128x644Rows);
                assert(lcd_display_ != nullptr);
            }
        }
        else
        {
            lcd_display_ = new Ssd1306(OledPanel::k128x648Rows);
            assert(lcd_display_ != nullptr);
        }

        if (lcd_display_->Start())
        {
            type_ = display::Type::kSsd1306;
            Printf(1, "SSD1306");
        }
    }
#if defined(CONFIG_DISPLAY_ENABLE_HD44780)
    else if (HAL_I2C::IsConnected(hd44780::pcf8574t::kTC2004Address))
    {
        lcd_display_ = new Hd44780(hd44780::pcf8574t::kTC2004Address, 20, 4);
        assert(lcd_display_ != nullptr);

        if (lcd_display_->Start())
        {
            type_ = display::Type::kPcf8574T2004;
            Printf(1, "TC2004_PCF8574T");
        }
    }
    else if (HAL_I2C::IsConnected(hd44780::pcf8574t::kTC1602Address))
    {
        lcd_display_ = new Hd44780(hd44780::pcf8574t::kTC1602Address, 16, 2);
        assert(lcd_display_ != nullptr);

        if (lcd_display_->Start())
        {
            type_ = display::Type::kPcf8574T1602;
            Printf(1, "TC1602_PCF8574T");
        }
    }
#endif

    if (lcd_display_ == nullptr)
    {
        sleep_timeout_ = 0;
    }
}
