/**
 * @file spi.cpp
 *
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

#include <cstdio>

#include "spi.h"

namespace spi {
void Begin() {
    puts("spi::Begin");
}

void SetSpeedHz(uint32_t speed_hz) {
    printf("spi::SetSpeedHz=%u\n", speed_hz);
}

void SetDataMode(uint8_t mode) {
    printf("spi::SetDataMode=%u\n", mode);
}

void ChipSelect(uint8_t chip_select) {
    printf("spi::ChipSelect=%u\n", chip_select);
}

void Transfern(char* tx_buffer, uint32_t length) {
    printf("spi::Transfern=%p:%u\n", reinterpret_cast<void*>(tx_buffer), length);
}

void Write(uint16_t data) {
    printf("spi::Write=%u\n", data);
}

void Writenb(const char* data, uint32_t length) {
    printf("spi::Writenb=%p:%u\n", reinterpret_cast<const void*>(data), length);
}
} // namespace spi