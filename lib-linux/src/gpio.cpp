/**
 * @file gpio.cpp
 * @brief Linux
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

#include <cstdint>
#include <cstdio>

#include "gpio.h"

namespace gpio {
void Fsel(uint32_t gpio, Select fsel) {
    printf("gpio::Fsel(gpio=%u,fsel=%u)\n", gpio, static_cast<unsigned>(fsel));
}

void Set(uint32_t pin) {
    printf("gpio::Set(%u)\n", pin);
}

void Clr(uint32_t pin) {
    printf("gpio::Clr(%u)\n", pin);
}

void Write(uint32_t pin, uint32_t value) {
    if (value != 0) {
        Set(pin);
    } else {
        Clr(pin);
    }
}

void SetPud(uint32_t gpio, Pull pull) {
    printf("gpio::SetPud(gpio=%u,pull=%u)\n", gpio, static_cast<unsigned>(pull));
}

void IntCfg(uint32_t gpio, IntConfig int_cfg) {
    printf("gpio::IntCfg(gpio=%u,int_cfg=%u)\n", gpio, static_cast<unsigned>(int_cfg));
}
} // namespace gpio