/**
 * @file max7219.h
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

#ifndef MAX7219_H_
#define MAX7219_H_

#include <cstdint>

#include "spi.h"

namespace max7219 {
inline constexpr uint32_t kSpeedMaxHz = 10000000;    // 10 MHz
inline constexpr uint32_t kSpeedDefaultHz = 2000000; // 2 MHz
//
inline constexpr uint32_t kMaX7219Ok = 0;
inline constexpr uint32_t kMaX7219Error = 1;
// https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
namespace reg {
inline constexpr uint8_t kNoop = 0x00;
inline constexpr uint8_t kDigit0 = 0x01;
inline constexpr uint8_t kDigit1 = 0x02;
inline constexpr uint8_t kDigit2 = 0x03;
inline constexpr uint8_t kDigit3 = 0x04;
inline constexpr uint8_t kDigit4 = 0x05;
inline constexpr uint8_t kDigit5 = 0x06;
inline constexpr uint8_t kDigit6 = 0x07;
inline constexpr uint8_t kDigit7 = 0x08;
inline constexpr uint8_t kDecodeMode = 0x09;
inline constexpr uint8_t kIntensity = 0x0A;
inline constexpr uint8_t kScanLimit = 0x0B;
inline constexpr uint8_t kShutdown = 0x0C;
inline constexpr uint8_t kDisplayTest = 0x0F;
namespace decode_mode {
inline constexpr uint8_t kCodeb = 0xFF; // Code B decode for digits 7–0
} // namespace decode_mode
namespace shutdown {
inline constexpr uint8_t kMode = 0x00;
inline constexpr uint8_t kNormalOp = 0x01;
} // namespace shutdown
} // namespace reg
namespace digit {
inline constexpr uint8_t kNegative = 0x0A;
inline constexpr uint8_t kE = 0x0B;
inline constexpr uint8_t kH = 0x0C;
inline constexpr uint8_t kL = 0x0D;
inline constexpr uint8_t kP = 0x0E;
inline constexpr uint8_t kBlank = 0x0F;
} // namespace digit
} // namespace max7219

class MAX7219 : public Spi {
   public:
    explicit MAX7219(uint32_t speed_hz = 0) : Spi(spi::kCs, speed_hz == 0 ? max7219::kSpeedDefaultHz : (speed_hz <= max7219::kSpeedMaxHz ? speed_hz : max7219::kSpeedMaxHz)) {}

    void WriteRegister(uint32_t reg, uint32_t data, bool spi_setup) {
        const auto kSpiData = static_cast<uint16_t>(((reg & 0xFF) << 8) | (data & 0xFF));
        Spi::Write(kSpiData, spi_setup);
    }
};

#endif // MAX7219_H_
