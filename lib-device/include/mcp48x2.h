/**
 * @file mcp48x2.h
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

#ifndef MCP48X2_H_
#define MCP48X2_H_

#include <cstdint>

#include "spi.h"

/*
 * MCP4802: Dual 8-Bit Voltage Output DAC
 * MCP4812: Dual 10-Bit Voltage Output DAC
 * MCP4822: Dual 12-Bit Voltage Output DAC
 */

namespace dac {
namespace mcp48x2 {
namespace speed {
inline constexpr uint32_t kMaxHz = 20000000;     ///< 20 MHz
inline constexpr uint32_t kDefaultHz = 10000000; ///< 10 MHz
} // namespace speed
namespace mask {
inline constexpr uint16_t kDacSelection = (1U << 15);
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
} // namespace mcp48x2

class MCP4822 : Spi {
   public:
    explicit MCP4822(uint8_t chip_select = 0, uint32_t speed_hz = mcp48x2::speed::kDefaultHz)
        : Spi(chip_select, (speed_hz == 0 ? mcp48x2::speed::kDefaultHz : (mcp48x2::speed::kDefaultHz <= mcp48x2::speed::kMaxHz ? speed_hz : mcp48x2::speed::kMaxHz))) {}

    void WriteDacA(uint16_t data) {
        const uint16_t kSpiData = mcp48x2::reg::kWriteDacA | mcp48x2::reg::kGain1x | mcp48x2::reg::kShutdownOff | Data12Bit(data);
        Spi::Write(kSpiData, true);
    }

    void WriteDacB(uint16_t data) {
        const uint16_t kSpiData = mcp48x2::reg::kWriteDacB | mcp48x2::reg::kGain1x | mcp48x2::reg::kShutdownOff | Data12Bit(data);
        Spi::Write(kSpiData, true);
    }

    void WriteDacAB(uint16_t data_a, uint16_t data_b) {
        const uint16_t kSpiDataA = mcp48x2::reg::kWriteDacA | mcp48x2::reg::kGain1x | mcp48x2::reg::kShutdownOff | Data12Bit(data_a);
        const uint16_t kSpiDataB = mcp48x2::reg::kWriteDacB | mcp48x2::reg::kGain1x | mcp48x2::reg::kShutdownOff | Data12Bit(data_b);
        Spi::Write(kSpiDataA, true);
        Spi::Write(kSpiDataB, false);
    }

   private:
    uint16_t Data12Bit(uint16_t data) { return (data & mcp48x2::mask::kData12bit) << mcp48x2::shift::kData12bit; }
};
} // namespace dac

#endif // MCP48X2_H_
