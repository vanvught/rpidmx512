/**
 * @file gpio.h
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

#ifndef GPIO_H_
#define GPIO_H_

#include <cstdint>

namespace gpio {
enum class Pull { kDisable, kUp, kDown };
enum class IntConfig { kPosEdge, kNegEdge, kHighLev, kLowLev, kDoubleEdge };

void Fsel(uint32_t gpio, uint32_t fsel);
void Set(uint32_t pin);
void Clr(uint32_t pin);
void Write(uint32_t pin, uint32_t value);
uint8_t Lev(uint32_t pin);
void SetPud(uint32_t gpio, Pull pull);
void IntCfg(uint32_t gpio, IntConfig int_cfg);
} // namespace gpio

#endif // GPIO_H_
