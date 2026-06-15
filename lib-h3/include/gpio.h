/**
 * @file gpio.h
 * @brief H2+/H3
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GPIO_H_
#define GPIO_H_

#include <cstdint>

#include "h3_gpio.h"
#include "h3_board.h" // IWYU pragma: keep

namespace gpio {
enum class Pull { 
	kDisable = GPIO_PULL_DISABLE, 
	kUp = GPIO_PULL_UP, 
	kDown = GPIO_PULL_DOWN 
};

enum class IntConfig {
	kPosEdge = GPIO_INT_CFG_POS_EDGE,
	kNegEdge = GPIO_INT_CFG_NEG_EDGE,
	kHighLev = GPIO_INT_CFG_HIGH_LEV,
	kLowLev = GPIO_INT_CFG_LOW_LEV,
	kDoubleEdge = GPIO_INT_CFG_DOUBLE_EDGE	
};

inline void Fsel(uint32_t gpio, uint32_t fsel) {
    H3GpioFsel(gpio, fsel);
}

inline void Set(uint32_t pin) {
    H3GpioSet(pin);
}

inline void Clr(uint32_t pin) {
    H3GpioClr(pin);
}

inline void Write(uint32_t pin, uint32_t value) {
    if (value != 0) {
        Set(pin);
    } else {
        Clr(pin);
    }
}

inline uint8_t Lev(uint32_t pin) {
    return H3GpioLev(pin);
}

inline void SetPud(uint32_t gpio, Pull pull) {
    H3GpioSetPud(gpio, static_cast<gpio_pull_t>(pull));
}

inline void IntCfg(uint32_t gpio, IntConfig int_cfg) {
	H3GpioIntCfg(gpio, static_cast<GpioIntCfg_t>(int_cfg));
}
} // namespace gpio

#endif // GPIO_H_
