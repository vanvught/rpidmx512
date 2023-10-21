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
#if defined (CONFIG_MCPBUTTONS_USE_MCP23X17)
# include "mcp23x17.h"
#endif

#include "debug.h"

namespace mcpbuttons {
namespace gpio {
#if defined (GD32)
///< INT = PA15
#else
static constexpr auto INT = GPIO_EXT_12;
#endif
}  // namespace gpio
namespace mcp23008 {
static constexpr uint8_t I2C_ADDRESS = 0x20;
}  // namespace mcp23008
namespace button {
static constexpr uint32_t ENTER = 2;
static constexpr uint32_t LEFT  = 3;
static constexpr uint32_t RIGHT = 4;
static constexpr uint32_t UP    = 5;
static constexpr uint32_t DOWN  = 6;
static constexpr uint32_t ESC   = 7;
}
static constexpr bool is_button_pressed(const uint32_t nButtonsChanged, const uint32_t nButton) {
	return ((nButtonsChanged & (1U << nButton)) == (1U << nButton));
}
}  // namespace mcpbuttons

class McpButtons {
public:
	McpButtons(bool bRotaryHalfStep) :
		m_I2C(mcpbuttons::mcp23008::I2C_ADDRESS),
		m_RotaryEncoder(bRotaryHalfStep),
		m_bIsConnected(m_I2C.IsConnected())
	{
		DEBUG_ENTRY

		if (!m_bIsConnected) {
			DEBUG_EXIT
			return;
		}

#if defined (CONFIG_MCPBUTTONS_USE_MCP23X17)
		m_I2C.WriteRegister(mcp23x17::REG_IOCON, mcp23x17::IOCON_BANK);
#endif
		m_I2C.WriteRegister(mcp23x08::REG_IODIR,   static_cast<uint8_t>(0xFF));		// All input
		m_I2C.WriteRegister(mcp23x08::REG_GPPU,    static_cast<uint8_t>(0xFF));		// Pull-up
		m_I2C.WriteRegister(mcp23x08::REG_IPOL,    static_cast<uint8_t>(0xFF));		// Invert read
		m_I2C.WriteRegister(mcp23x08::REG_INTCON,  static_cast<uint8_t>(0x00));
		m_I2C.WriteRegister(mcp23x08::REG_GPINTEN, static_cast<uint8_t>(0xFF));		// Interrupt on Change
		m_I2C.ReadRegister(mcp23x08::REG_INTCAP);									// Clear interrupts

#if defined (GD32)
		rcu_periph_clock_enable(RCU_GPIOA);
# if !defined(GD32F4XX)
		gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
	    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOD, GPIO_PIN_SOURCE_15);
# else
		gpio_af_set(GPIOD, GPIO_AF_0, GPIO_PIN_15);
	    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	    syscfg_exti_line_config(EXTI_SOURCE_GPIOD, EXTI_SOURCE_PIN15);
# endif
#else
		FUNC_PREFIX(gpio_fsel(mcpbuttons::gpio::INT, GPIO_FSEL_INPUT));
		FUNC_PREFIX(gpio_set_pud(mcpbuttons::gpio::INT, GPIO_PULL_UP));
#endif
		DEBUG_EXIT
	}

	bool IsConnected() const {
		return m_bIsConnected;
	}

	bool Start() const {
		return true;
	}

	bool IsAvailable() {
#if defined (GD32)
		extern volatile bool gv_buttons_irq;

		if (__builtin_expect((gv_buttons_irq), 0)) {
			gv_buttons_irq = false;
			return false;
		}
#else
		if ((!m_bIsConnected) && (__builtin_expect(FUNC_PREFIX(gpio_lev(mcpbuttons::gpio::INT)) == HIGH, 1))) {
			return false;
		}
#endif

		const auto nPort = m_I2C.ReadRegister(mcp23x08::REG_GPIO);
		const auto nButtonsChanged = static_cast<uint8_t>((nPort ^ m_nPortPrevious) & nPort);

		m_nPortPrevious = nPort;

//		printf("%.2x %.2x\r", nPort, nButtonsChanged);

		/* P = m_nPortPrevious
		 * N = m_nPort
		 * X = m_nPort ^ m_nPortPrevious
		 * C = nButtonsChanged
		 *
		 * P N	X N	C
		 * 0 0	0 0	0
		 * 0 1	1 1	1
		 * 1 0	1 0	0
		 * 1 1	0 1	0
		 */

		m_nKey = input::KEY_NOT_DEFINED;

		if (nButtonsChanged != 0) {
			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::ENTER)) {
				m_nKey = input::KEY_ENTER;
				return true;
			}

			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::LEFT)) {
				m_nKey = input::KEY_LEFT;
				return true;
			}

			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::RIGHT)) {
				m_nKey = input::KEY_RIGHT;
				return true;
			}

			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::UP)) {
				m_nKey = input::KEY_UP;
				return true;
			}

			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::DOWN)) {
				m_nKey = input::KEY_DOWN;
				return true;
			}

			if (mcpbuttons::is_button_pressed(nButtonsChanged, mcpbuttons::button::ESC)) {
				m_nKey = input::KEY_ESC;
				return true;
			}
		}

		const auto rotaryDirection = m_RotaryEncoder.Process(nPort);

		if (rotaryDirection == RotaryEncoder::CW) {
			m_nKey = input::KEY_UP;
		} else if (rotaryDirection == RotaryEncoder::CCW) {
			m_nKey = input::KEY_DOWN;
		}

		return (m_nKey != input::KEY_NOT_DEFINED);
	}

	int GetChar() {
		const auto nKey = m_nKey;
		m_nKey = input::KEY_NOT_DEFINED;
		return nKey;
	}

private:
	HAL_I2C m_I2C;
	RotaryEncoder m_RotaryEncoder;
	int m_nKey { input::KEY_NOT_DEFINED };
	bool m_bIsConnected;
	uint8_t m_nPortPrevious { 0 };
};

#endif /* MCPBUTTONS_H_ */
