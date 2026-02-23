/**
 * @file mcpbuttons.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MCPBUTTONS_H_
#define MCPBUTTONS_H_

#include <cstdint>

#include "input.h"
#include "rotaryencoder.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "mcp23x08.h"
#if defined(CONFIG_MCPBUTTONS_USE_MCP23X17)
#include "mcp23x17.h"
#endif
 #include "firmware/debug/debug_debug.h"

namespace mcpbuttons
{
namespace gpio
{
#if defined(GD32)
///< INT = PA15
#else
inline constexpr auto kInt = GPIO_EXT_12;
#endif
} // namespace gpio
namespace mcp23008
{
inline constexpr uint8_t kI2CAddress = 0x20;
} // namespace mcp23008
namespace button
{
inline constexpr uint32_t kEnter = 2;
inline constexpr uint32_t kLeft = 3;
inline constexpr uint32_t kRight = 4;
inline constexpr uint32_t kUp = 5;
inline constexpr uint32_t kDown = 6;
inline constexpr uint32_t kEsc = 7;
} // namespace button
inline constexpr bool IsButtonPressed(uint32_t buttons_changed, uint32_t button)
{
    return ((buttons_changed & (1U << button)) == (1U << button));
}
} // namespace mcpbuttons

class McpButtons
{
   public:
    explicit McpButtons(bool rotary_half_step)
        : hal_i2c_(mcpbuttons::mcp23008::kI2CAddress), rotary_encoder_(rotary_half_step), is_connected_(hal_i2c_.IsConnected())
    {
        DEBUG_ENTRY();

        if (!is_connected_)
        {
            DEBUG_EXIT();
            return;
        }

#if defined(CONFIG_MCPBUTTONS_USE_MCP23X17)
        hal_i2c_.WriteRegister(mcp23x17::REG_IOCON, mcp23x17::IOCON_BANK);
#endif
        hal_i2c_.WriteRegister(mcp23x08::REG_IODIR, static_cast<uint8_t>(0xFF)); // All input
        hal_i2c_.WriteRegister(mcp23x08::REG_GPPU, static_cast<uint8_t>(0xFF));  // Pull-up
        hal_i2c_.WriteRegister(mcp23x08::REG_IPOL, static_cast<uint8_t>(0xFF));  // Invert read
        hal_i2c_.WriteRegister(mcp23x08::REG_INTCON, static_cast<uint8_t>(0x00));
        hal_i2c_.WriteRegister(mcp23x08::REG_GPINTEN, static_cast<uint8_t>(0xFF)); // Interrupt on Change
        hal_i2c_.ReadRegister(mcp23x08::REG_INTCAP);                               // Clear interrupts

#if defined(GD32)
        rcu_periph_clock_enable(RCU_GPIOA);
#if !defined(GD32F4XX)
        gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
        gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_15);
#else
        gpio_af_set(GPIOD, GPIO_AF_0, GPIO_PIN_15);
        gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
        syscfg_exti_line_config(EXTI_SOURCE_GPIOD, EXTI_SOURCE_PIN15);
#endif
#else
        FUNC_PREFIX(GpioFsel(mcpbuttons::gpio::kInt, GPIO_FSEL_INPUT));
        FUNC_PREFIX(GpioSetPud(mcpbuttons::gpio::kInt, GPIO_PULL_UP));
#endif
        DEBUG_EXIT();
    }

    bool IsConnected() const { return is_connected_; }

    bool Start() const { return true; }

    bool IsAvailable()
    {
#if defined(GD32)
        extern volatile bool gv_buttons_irq;

        if (__builtin_expect((gv_buttons_irq), 0))
        {
            gv_buttons_irq = false;
            return false;
        }
#else
        if ((!is_connected_) && (__builtin_expect(FUNC_PREFIX(GpioLev(mcpbuttons::gpio::kInt)) == HIGH, 1)))
        {
            return false;
        }
#endif

        const auto kPort = hal_i2c_.ReadRegister(mcp23x08::REG_GPIO);
        const auto kButtonsChanged = static_cast<uint8_t>((kPort ^ port_previous_) & kPort);

        port_previous_ = kPort;

        //		printf("%.2x %.2x\r", kPort, kButtonsChanged);

        /* P = port_previous_
         * N = kPort
         * X = kPort ^ port_previous_
         * C = kButtonsChanged
         *
         * P N	X N	C
         * 0 0	0 0	0
         * 0 1	1 1	1
         * 1 0	1 0	0
         * 1 1	0 1	0
         */

        key_ = input::KEY_NOT_DEFINED;

        if (kButtonsChanged != 0)
        {
            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kEnter))
            {
                key_ = input::KEY_ENTER;
                return true;
            }

            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kLeft))
            {
                key_ = input::KEY_LEFT;
                return true;
            }

            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kRight))
            {
                key_ = input::KEY_RIGHT;
                return true;
            }

            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kUp))
            {
                key_ = input::KEY_UP;
                return true;
            }

            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kDown))
            {
                key_ = input::KEY_DOWN;
                return true;
            }

            if (mcpbuttons::IsButtonPressed(kButtonsChanged, mcpbuttons::button::kEsc))
            {
                key_ = input::KEY_ESC;
                return true;
            }
        }

        const auto kRotaryDirection = rotary_encoder_.Process(kPort);

        if (kRotaryDirection == RotaryEncoder::CW)
        {
            key_ = input::KEY_UP;
        }
        else if (kRotaryDirection == RotaryEncoder::CCW)
        {
            key_ = input::KEY_DOWN;
        }

        return (key_ != input::KEY_NOT_DEFINED);
    }

    int GetChar()
    {
        const auto kKey = key_;
        key_ = input::KEY_NOT_DEFINED;
        return kKey;
    }

   private:
    HAL_I2C hal_i2c_;
    RotaryEncoder rotary_encoder_;
    int key_{input::KEY_NOT_DEFINED};
    bool is_connected_;
    uint8_t port_previous_{0};
};

#endif  // MCPBUTTONS_H_
