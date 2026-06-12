/**
 * @file mcp23s17.h
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

#ifndef MCP23S17_H_
#define MCP23S17_H_

#include <cstdint>

#include "spi.h"
#include "mcp23x17.h"

namespace gpio {

class MCP23S17 : Spi {
   public:
    explicit MCP23S17(uint8_t chip_select = 0, uint32_t speed_hz = mcp23x17::SPI_SPEED_DEFAULT_HZ, uint8_t address = 0)
        : Spi(chip_select, (speed_hz == 0 ? mcp23x17::SPI_SPEED_DEFAULT_HZ : (mcp23x17::SPI_SPEED_DEFAULT_HZ <= mcp23x17::SPI_SPEED_MAX_HZ ? speed_hz : mcp23x17::SPI_SPEED_MAX_HZ))), address_(address) {
        MCP23S17::WriteRegister(mcp23x17::REG_IOCON, mcp23x17::IOCON_HAEN);
    }

    void WriteRegister(uint8_t reg, uint8_t value) {
        char spi_data[3];

        spi_data[0] = static_cast<char>(mcp23x17::SPI_CMD_WRITE) | static_cast<char>(address_ << 1);
        spi_data[1] = static_cast<char>(reg);
        spi_data[2] = static_cast<char>(value);

        Spi::Write(spi_data, 3, true);
    }

    void WriteRegister(uint8_t reg, uint16_t value) {
        char spi_data[4];

        spi_data[0] = static_cast<char>(mcp23x17::SPI_CMD_WRITE) | static_cast<char>(address_ << 1);
        spi_data[1] = static_cast<char>(reg);
        spi_data[2] = static_cast<char>(value);
        spi_data[3] = static_cast<char>(value >> 8);

        Spi::Write(spi_data, 4, true);
    }

   private:
    void Setup() {
        spi::ChipSelect(chip_select_);
        spi::SetDataMode(spi::kMode0);
        spi::SetSpeedHz(speed_hz_);
    }

   private:
    uint8_t chip_select_{0};
    uint32_t speed_hz_{0};
    uint8_t address_{0};
};

} // namespace gpio

#endif // MCP23S17_H_
