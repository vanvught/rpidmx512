/**
 * @file buttonsmcp.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "buttonsmcp.h"
#include "oscclient.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "mcp23x17.h"

 #include "firmware/debug/debug_debug.h"

namespace mcp23017
{
static constexpr auto kAddress = 0x20;
}

namespace gpio
{
static constexpr auto kInterrupt = GPIO_EXT_12; // PA7
}

ButtonsMcp::ButtonsMcp(OscClient* pOscClient) : hal_i2c_(mcp23017::kAddress), oscclient_(pOscClient)
{
    assert(oscclient_ != nullptr);
}

bool ButtonsMcp::Start()
{
    DEBUG_ENTRY();

    is_connected_ = hal_i2c_.IsConnected();

    if (!is_connected_)
    {
        DEBUG_EXIT();
        return false;
    }

    // Switches
    hal_i2c_.WriteRegister(mcp23x17::REG_IODIRA, static_cast<uint8_t>(0xFF)); // All input
    hal_i2c_.WriteRegister(mcp23x17::REG_GPPUA, static_cast<uint8_t>(0xFF));  // Pull-up
    hal_i2c_.WriteRegister(mcp23x17::REG_IPOLA, static_cast<uint8_t>(0xFF));  // Invert read
    hal_i2c_.WriteRegister(mcp23x17::REG_INTCONA, static_cast<uint8_t>(0x00));
    hal_i2c_.WriteRegister(mcp23x17::REG_GPINTENA, static_cast<uint8_t>(0xFF)); // Interrupt on Change
    hal_i2c_.ReadRegister(mcp23x17::REG_INTCAPA);                               // Clear interrupt
    // Led's
    hal_i2c_.WriteRegister(mcp23x17::REG_IODIRB, static_cast<uint8_t>(0x00)); // All output
    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, static_cast<uint8_t>(0x00));  // All led's Off

    FUNC_PREFIX(GpioFsel(gpio::kInterrupt, GPIO_FSEL_INPUT));
    FUNC_PREFIX(GpioSetPud(gpio::kInterrupt, GPIO_PULL_UP));

    buttons_count_ = 8;

    DEBUG_EXIT();
    return true;
}

void ButtonsMcp::Stop()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void ButtonsMcp::Run()
{
    if (__builtin_expect(FUNC_PREFIX(GpioLev(gpio::kInterrupt)) == LOW, 0))
    {
        const auto buttons = hal_i2c_.ReadRegister(mcp23x17::REG_GPIOA);
        const uint8_t buttons_changed = (buttons ^ buttons_previous_) & buttons;

        buttons_previous_ = buttons;

        /* P = buttons_previous_
         * N = buttons_
         * X = buttons_ ^ buttons_previous_
         * C = buttons_changed
         *
         * P N	X N	C
         * 0 0	0 0	0
         * 0 1	1 1	1
         * 1 0	1 0	0
         * 1 1	0 1	0
         */

        DEBUG_PRINTF("%.2x %.2x", buttons, buttons_changed);

        for (uint32_t i = 0; i < 8; i++)
        {
            if ((buttons_changed & (1U << i)) == ((1U << i)))
            {
                oscclient_->SendCmd(i);
            }
        }
    }
}

void ButtonsMcp::SetLed(uint32_t led, bool on)
{
    DEBUG_PRINTF("led%d %s", led, on ? "On" : "Off");

    port_b_ &= static_cast<uint8_t>(~(1U << led));

    if (on)
    {
        port_b_ |= static_cast<uint8_t>(1U << led);
    }

    hal_i2c_.WriteRegister(mcp23x17::REG_GPIOB, port_b_);

    DEBUG_PRINTF("%.2x", port_b_);
}
