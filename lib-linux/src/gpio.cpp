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

#include <cstddef>
#include <cstdint>
#include <cstdio>
#if defined(__linux__)
#include <gpiod.h>
#include <unistd.h>
#include <cassert>
#endif

#include "gpio.h"

namespace gpio {
#if defined(__linux__)
static auto s_init = false;

const char* gpio_chip = "gpiochip0";

static struct gpiod_chip* s_chip;
static struct gpiod_line* s_line[100];

static constexpr auto kMakLines = sizeof(s_line) / sizeof(s_line[0]);

static bool GetLine(uint32_t line_number) {
    assert(line_number < kMaxLines);

    if (s_line[line_number]) {
        return true;
    }

    s_line[line_number] = gpiod_chip_get_line(s_chip, line_number);

    if (!s_line[line_number]) {
        perror("GetLine");
        return false;
    }

    return true;
}

static void Init() {
    s_chip = gpiod_chip_open_by_name(gpio_chip);

    if (!s_chip) {
        perror("gpio::Init");
        return;
    }

    for (size_t i = 0; i < kMakLines; i++) {
        s_line[i] = nullptr;
    }

    s_init = true;

    puts("gpio::Init");
}
#endif

void Fsel(uint32_t gpio, Select fsel) {
#if defined(__linux__)
    if (!s_init) [[unlikely]] {
        Init();
    }
#endif
    printf("gpio::Fsel(gpio=%u,fsel=%u)\n", gpio, static_cast<unsigned>(fsel));
#if defined(__linux__)
    if (GetLine(gpio)) {
        switch (fsel) {
            case Select::kInput:
                if (gpiod_line_request_input(s_line[gpio], "kInput") < 0) {
                    perror("gpio::Fsel gpiod_line_request_input");
                }
                return;
                break;
            case Select::kOutput:
                if (gpiod_line_request_output(s_line[gpio], "kOutput", 0) < 0) {
                    perror("gpio::Fsel gpiod_line_request_output");
                }
                return;
                break;
            case Select::kEint:
                break;
            case Select::kDisable:
                break;
        }
    }
#endif
}

void SetPud(uint32_t gpio, Pull pull) {
#if defined(__linux__)
    if (!s_init) [[unlikely]] {
        Init();
    }
#endif
    printf("gpio::SetPud(gpio=%u,pull=%u)\n", gpio, static_cast<unsigned>(pull));
}

void IntCfg(uint32_t gpio, IntConfig int_cfg) {
#if defined(__linux__)
    if (!s_init) [[unlikely]] {
        Init();
    }
#endif
    printf("gpio::IntCfg(gpio=%u,int_cfg=%u)\n", gpio, static_cast<unsigned>(int_cfg));
}

void Set(uint32_t pin) {
#if defined(__linux__)
    if (s_line[pin]) {
        gpiod_line_set_value(s_line[pin], 1);
        return;
    }
#endif
    printf("gpio::Set(%u)\n", pin);
}

void Clr(uint32_t pin) {
#if defined(__linux__)
    if (s_line[pin]) {
        gpiod_line_set_value(s_line[pin], 0);
        return;
    }
#endif
    printf("gpio::Clr(%u)\n", pin);
}

void Write(uint32_t pin, uint32_t value) {
    if (value != 0) {
        Set(pin);
    } else {
        Clr(pin);
    }
}

uint8_t Lev(uint32_t pin) {
#if defined(__linux__)
    if (s_line[pin]) {
        const auto kVal = gpiod_line_get_value(s_line[pin]);
        return static_cast<uint8_t>(kVal);
    }
#endif
    printf("gpio::Lev(pin=%u)\n", pin);
    return UINT8_MAX;
}
} // namespace gpio