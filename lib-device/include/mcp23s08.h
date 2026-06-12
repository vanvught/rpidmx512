/**
 * @file mcp23s08.h
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

#ifndef MCP23S08_H_
#define MCP23S08_H_

#include <cstdint>

#include "spi.h"

namespace gpio {
namespace mcp23s08 {
namespace speed {
inline constexpr uint32_t kMaxHz = 10000000;    ///< 10 MHz
inline constexpr uint32_t kDefaultHz = 2000000; ///< 2 MHz
} // namespace speed
namespace reg {
inline constexpr uint8_t kIodir = 0x00;   ///< I/O DIRECTION (IODIR) REGISTER
inline constexpr uint8_t kIpol = 0x01;    ///< INPUT POLARITY (IPOL) REGISTER
inline constexpr uint8_t kGpinten = 0x02; ///< INTERRUPT-ON-CHANGE CONTROL (GPINTEN) REGISTER
inline constexpr uint8_t kDefval = 0x03;  ///< DEFAULT COMPARE (DEFVAL) REGISTER FOR INTERRUPT-ON-CHANGE
inline constexpr uint8_t kIntcon = 0x04;  ///< INTERRUPT CONTROL (INTCON) REGISTER
inline constexpr uint8_t kIocon = 0x05;   ///< CONFIGURATION (IOCON) REGISTER
inline constexpr uint8_t kGppu = 0x06;    ///< PULL-UP RESISTOR CONFIGURATION (GPPU) REGISTER
inline constexpr uint8_t kIntf = 0x07;    ///< INTERRUPT FLAG (INTF) REGISTER
inline constexpr uint8_t kIntcap = 0x08;  ///< INTERRUPT CAPTURE (INTCAP) REGISTER
inline constexpr uint8_t kGpio = 0x09;    ///< PORT (GPIO) REGISTER
inline constexpr uint8_t kOlat = 0x0A;    ///< OUTPUT LATCH REGISTER (OLAT)
namespace iocon {
inline constexpr uint8_t kHaen = (1U << 3);
} // namespace iocon
} // namespace reg
namespace cmd {
inline constexpr uint8_t kWrite = 0x40;
inline constexpr uint8_t kRead = 0x41;
} // namespace cmd
} // namespace mcp23s08

class MCP23S08 : Spi {
   public:
    explicit MCP23S08(uint8_t chip_select = 0, uint32_t speed_hz = mcp23s08::speed::kDefaultHz, uint8_t address = 0)
        : Spi(chip_select, (speed_hz == 0 ? mcp23s08::speed::kDefaultHz : (mcp23s08::speed::kDefaultHz <= mcp23s08::speed::kMaxHz ? speed_hz : mcp23s08::speed::kMaxHz))), address_(address) {
        MCP23S08::WriteRegister(mcp23s08::reg::kIocon, mcp23s08::reg::iocon::kHaen);
    }

    void WriteRegister(uint8_t reg, uint8_t value) {
        char spi_data[3];

        spi_data[0] = static_cast<char>(mcp23s08::cmd::kWrite) | static_cast<char>(address_ << 1);
        spi_data[1] = static_cast<char>(reg);
        spi_data[2] = static_cast<char>(value);

        Spi::Write(spi_data, 3, true);
    }

   private:
    uint8_t address_{0};
};
} // namespace gpio

#endif // MCP23S08_H_
