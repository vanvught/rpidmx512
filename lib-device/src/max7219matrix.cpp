/**
 * @file max7219matrix.cpp
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <algorithm>

#include "max7219matrix.h"
#include "font_cp437.h"
#include "firmware/debug/debug_debug.h"

static uint8_t spi_data[64] __attribute__((aligned(4)));
static constexpr auto kFontSize = Cp437FontSize();
static uint8_t s_font[kFontSize] __attribute__((aligned(4)));

static constexpr uint8_t Rotate(uint32_t r, uint32_t x) {
    uint8_t byte = 0;

    for (uint32_t y = 0; y < 8; y++) {
        const auto kSet = kCp437Font[r][y] & (1U << x);
        byte |= static_cast<uint8_t>((kSet != 0) ? (1U << y) : 0);
    }

    return byte;
}

Max7219Matrix::Max7219Matrix() {
    DEBUG_ENTRY();

    auto dst = s_font;

    for (uint32_t i = 0; i < kFontSize; i++) {
        for (uint32_t j = 0; j < 8; j++) {
            *dst++ = Rotate(i, 7 - j);
        }
    }

    DEBUG_EXIT();
}

Max7219Matrix::~Max7219Matrix() {
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

void Max7219Matrix::Init(uint16_t count, uint8_t intensity) {
    DEBUG_ENTRY();

    constexpr uint16_t kSf = sizeof(spi_data) / 2;
    count_ = std::min(count, kSf);

    DEBUG_PRINTF("count_=%d", count_);

    WriteAll(max7219::reg::kShutdown, max7219::reg::shutdown::kNormalOp);
    WriteAll(max7219::reg::kDisplayTest, 0);
    WriteAll(max7219::reg::kDecodeMode, 0);
    WriteAll(max7219::reg::kScanLimit, 7);

    SetIntensity(intensity);

    Max7219Matrix::Cls();

    DEBUG_EXIT();
}

void Max7219Matrix::Write(const char* buffer, uint16_t count) {
    DEBUG_PRINTF("count=%d", count);

    if (count > count_) {
        count = count_;
    }

    int32_t k;

    for (uint32_t i = 1; i < 9; i++) {
        k = static_cast<int32_t>(count);

        uint16_t j;

        for (j = 0; j < (count_ * 2U) - (count * 2U); j = static_cast<uint16_t>(j + 2)) {
            spi_data[j] = max7219::reg::kNoop;
            spi_data[j + 1] = 0;
        }

        while (--k >= 0) {
            auto c = static_cast<uint32_t>(buffer[k]);

            if (c >= kFontSize) {
                c = ' ';
            }

            const auto kP = &s_font[c * 8];

            spi_data[j++] = static_cast<uint8_t>(i);
            spi_data[j++] = kP[i - 1];
        }

        Spi::Write(reinterpret_cast<const char*>(spi_data), j, true);
    }
}

void Max7219Matrix::UpdateCharacter(uint32_t c, const uint8_t bytes[8]) {
    if (c > kFontSize) {
        return;
    }

    auto font = &s_font[c * 8];

    for (uint32_t j = 0; j < 8; j++) {
        uint8_t b = 0;

        for (uint32_t y = 0; y < 8; y++) {
            const auto kSet = bytes[y] & (1U << (7U - j));
            b |= static_cast<uint8_t>((kSet != 0) ? (1U << y) : 0);
        }

        font[j] = b;
    }
}

void Max7219Matrix::WriteAll(uint8_t reg, uint8_t data) {
    DEBUG_ENTRY();

    for (uint32_t i = 0; i < (count_ * 2); i = i + 2) {
        spi_data[i] = reg;
        spi_data[i + 1] = data;
    }

    Spi::Write(reinterpret_cast<const char*>(spi_data), static_cast<uint32_t>(count_ * 2), true);

    DEBUG_EXIT();
}
