/**
 * @file mcp23s08.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal_spi.h"

namespace gpio
{
namespace mcp23s08
{
namespace speed
{
static constexpr uint32_t max_hz = 10000000;    ///< 10 MHz
static constexpr uint32_t default_hz = 2000000; ///< 2 MHz
} // namespace speed
namespace reg
{
static constexpr uint8_t IODIR = 0x00;   ///< I/O DIRECTION (IODIR) REGISTER
static constexpr uint8_t IPOL = 0x01;    ///< INPUT POLARITY (IPOL) REGISTER
static constexpr uint8_t GPINTEN = 0x02; ///< INTERRUPT-ON-CHANGE CONTROL (GPINTEN) REGISTER
static constexpr uint8_t DEFVAL = 0x03;  ///< DEFAULT COMPARE (DEFVAL) REGISTER FOR INTERRUPT-ON-CHANGE
static constexpr uint8_t INTCON = 0x04;  ///< INTERRUPT CONTROL (INTCON) REGISTER
static constexpr uint8_t IOCON = 0x05;   ///< CONFIGURATION (IOCON) REGISTER
static constexpr uint8_t GPPU = 0x06;    ///< PULL-UP RESISTOR CONFIGURATION (GPPU) REGISTER
static constexpr uint8_t INTF = 0x07;    ///< INTERRUPT FLAG (INTF) REGISTER
static constexpr uint8_t INTCAP = 0x08;  ///< INTERRUPT CAPTURE (INTCAP) REGISTER
static constexpr uint8_t GPIO = 0x09;    ///< PORT (GPIO) REGISTER
static constexpr uint8_t OLAT = 0x0A;    ///< OUTPUT LATCH REGISTER (OLAT)
namespace iocon
{
static constexpr uint8_t HAEN = (1U << 3);
} // namespace iocon
} // namespace reg
namespace cmd
{
static constexpr uint8_t WRITE = 0x40;
static constexpr uint8_t READ = 0x41;
} // namespace cmd
} // namespace mcp23s08

class MCP23S08 : HAL_SPI
{
   public:
    MCP23S08(uint8_t nChipSelect = 0, uint32_t nSpeedHz = mcp23s08::speed::default_hz, uint8_t address = 0)
        : HAL_SPI(nChipSelect,
                  nSpeedHz == 0 ? mcp23s08::speed::default_hz : (mcp23s08::speed::default_hz <= mcp23s08::speed::max_hz ? nSpeedHz : mcp23s08::speed::max_hz)),
          address_(address)
    {
        MCP23S08::WriteRegister(mcp23s08::reg::IOCON, mcp23s08::reg::iocon::HAEN);
    }

    void WriteRegister(uint8_t nRegister, uint8_t nValue)
    {
        char spiData[3];

        spiData[0] = static_cast<char>(mcp23s08::cmd::WRITE) | static_cast<char>(address_ << 1);
        spiData[1] = static_cast<char>(nRegister);
        spiData[2] = static_cast<char>(nValue);

        HAL_SPI::Write(spiData, 3);
    }

   private:
    uint8_t address_ = 0;
};

} // namespace gpio

#endif  // MCP23S08_H_
