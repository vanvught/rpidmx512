/**
 * @file max7219.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal_spi.h"

namespace max7219
{
static constexpr uint32_t SPEED_MAX_HZ = 10000000;    // 10 MHz
static constexpr uint32_t SPEED_DEFAULT_HZ = 2000000; // 2 MHz
//
static constexpr uint32_t MAX7219_OK = 0;
static constexpr uint32_t MAX7219_ERROR = 1;
// https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
namespace reg
{
static constexpr uint8_t NOOP = 0x00;
static constexpr uint8_t DIGIT0 = 0x01;
static constexpr uint8_t DIGIT1 = 0x02;
static constexpr uint8_t DIGIT2 = 0x03;
static constexpr uint8_t DIGIT3 = 0x04;
static constexpr uint8_t DIGIT4 = 0x05;
static constexpr uint8_t DIGIT5 = 0x06;
static constexpr uint8_t DIGIT6 = 0x07;
static constexpr uint8_t DIGIT7 = 0x08;
static constexpr uint8_t DECODE_MODE = 0x09;
static constexpr uint8_t INTENSITY = 0x0A;
static constexpr uint8_t SCAN_LIMIT = 0x0B;
static constexpr uint8_t SHUTDOWN = 0x0C;
static constexpr uint8_t DISPLAY_TEST = 0x0F;
namespace decode_mode
{
static constexpr uint8_t CODEB = 0xFF; // Code B decode for digits 7–0
} // namespace decode_mode
namespace shutdown
{
static constexpr uint8_t MODE = 0x00;
static constexpr uint8_t NORMAL_OP = 0x01;
} // namespace shutdown
} // namespace reg
namespace digit
{
static constexpr uint8_t NEGATIVE = 0x0A;
static constexpr uint8_t E = 0x0B;
static constexpr uint8_t H = 0x0C;
static constexpr uint8_t L = 0x0D;
static constexpr uint8_t P = 0x0E;
static constexpr uint8_t BLANK = 0x0F;
} // namespace digit
} // namespace max7219

class MAX7219 : public HAL_SPI
{
   public:
    explicit MAX7219(uint32_t speed_hz = 0)
        : HAL_SPI(SPI_CS0, speed_hz == 0 ? max7219::SPEED_DEFAULT_HZ : (speed_hz <= max7219::SPEED_MAX_HZ ? speed_hz : max7219::SPEED_MAX_HZ))
    {
    }

    void WriteRegister(uint32_t reg, uint32_t data, bool spi_setup = true)
    {
        const auto kSpiData = static_cast<uint16_t>(((reg & 0xFF) << 8) | (data & 0xFF));
        Write(kSpiData, spi_setup);
    }
};

#endif  // MAX7219_H_
