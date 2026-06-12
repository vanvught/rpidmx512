/**
 * @file mcp49x2.h
 *
 */
/* Copyright (C) 2014-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MCP49X2_H_
#define MCP49X2_H_

#include <cstdint>

#include "spi.h"

/*
 * MCP4902: Dual 8-Bit Voltage Output DAC
 * MCP4912: Dual 10-Bit Voltage Output DAC
 * MCP4922: Dual 12-Bit Voltage Output DAC
 */

namespace dac {
namespace mcp49x2 {
namespace speed {
inline constexpr uint32_t kMaxHz = 20000000;     ///< 20 MHz
inline constexpr uint32_t kDefaultHz = 10000000; ///< 10 MHz
} // namespace speed
namespace mask {
inline constexpr uint16_t kDacSelection = (1U << 15);
inline constexpr uint16_t kBuffer = (1U << 14);
inline constexpr uint16_t kGain = (1U << 13);
inline constexpr uint16_t kShutdown = (1U << 12);
inline constexpr uint16_t kData12bit = 0x0FFF;
inline constexpr uint16_t kData10bit = 0x03FF;
inline constexpr uint16_t kData8bit = 0x00FF;
} // namespace mask
namespace shift {
inline constexpr uint16_t kData12bit = 0;
inline constexpr uint16_t kData10bit = 2;
inline constexpr uint16_t kData8bit = 4;
} // namespace shift
namespace reg {
inline constexpr uint16_t kWriteDacA = (0 << 15);
inline constexpr uint16_t kWriteDacB = (1U << 15);
inline constexpr uint16_t kGain2x = (0 << 13);
inline constexpr uint16_t kGain1x = (1U << 13);
inline constexpr uint16_t kShutdownOn = (0 << 12);
inline constexpr uint16_t kShutdownOff = (1U << 12);
} // namespace reg
} // namespace mcp49x2

class MCP4902 : Spi {
   public:
    explicit MCP4902(uint8_t chip_select = 0, uint32_t speed_hz = mcp49x2::speed::kDefaultHz)
        : Spi(chip_select, (speed_hz == 0 ? mcp49x2::speed::kDefaultHz : (mcp49x2::speed::kDefaultHz <= mcp49x2::speed::kMaxHz ? speed_hz : mcp49x2::speed::kMaxHz))) {}

    void WriteDacA(uint8_t data) {
        const uint16_t kSpiData = mcp49x2::reg::kWriteDacA | mcp49x2::reg::kGain1x | mcp49x2::reg::kShutdownOff | Data8Bit(data);
        Spi::Write(kSpiData, true);
    }

    void WriteDacB(uint8_t data) {
        const uint16_t kSpiData = mcp49x2::reg::kWriteDacB | mcp49x2::reg::kGain1x | mcp49x2::reg::kShutdownOff | Data8Bit(data);
        Spi::Write(kSpiData, true);
    }

    void WriteDacAB(uint8_t data_a, uint8_t data_b) {
        const uint16_t kSpiDataA = mcp49x2::reg::kWriteDacA | mcp49x2::reg::kGain1x | mcp49x2::reg::kShutdownOff | Data8Bit(data_a);
        const uint16_t kSpiDataB = mcp49x2::reg::kWriteDacB | mcp49x2::reg::kGain1x | mcp49x2::reg::kShutdownOff | Data8Bit(data_b);
        Spi::Write(kSpiDataA, true);
        Spi::Write(kSpiDataB, false);
    }

   private:
    uint16_t Data8Bit(uint8_t data) { return static_cast<uint16_t>((data & mcp49x2::mask::kData8bit) << mcp49x2::shift::kData8bit); }
};
} // namespace dac

#endif // MCP49X2_H_
