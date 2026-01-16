/**
 * @file mcp23x17.h
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

#ifndef MCP23X17_H_
#define MCP23X17_H_

#include <cstdint>

namespace mcp23x17
{
///< IOCON.BANK = 0
static constexpr uint8_t REG_IODIRA = 0x00; ///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
static constexpr uint8_t REG_IODIRB = 0x01; ///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
static constexpr uint8_t REG_IPOLA = 0x02;  ///< INPUT POLARITY (IPOLA) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
static constexpr uint8_t REG_IPOLB = 0x03;  ///< INPUT POLARITY (IPOLB) REGISTER, 0 = Normal (default)(low reads as 0), 1 = Inverted (low reads as 1)
static constexpr uint8_t REG_GPINTENA =
    0x04; ///< INTERRUPT-ON-CHANGE CONTROL (GPINTENA) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
static constexpr uint8_t REG_GPINTENB =
    0x05; ///< INTERRUPT-ON-CHANGE CONTROL (GPINTENB) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
static constexpr uint8_t REG_DEFVALA =
    0x06; ///< DEFAULT COMPARE (DEFVALA) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
static constexpr uint8_t REG_DEFVALB =
    0x07; ///< DEFAULT COMPARE (DEFVALB) REGISTER FOR INTERRUPT-ON-CHANGE, Opposite of what is here will trigger an interrupt (default = 0)
static constexpr uint8_t REG_INTCONA =
    0x08; ///< INTERRUPT CONTROL (INTCONA) REGISTER, 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
static constexpr uint8_t REG_INTCONB =
    0x09; ///< INTERRUPT CONTROL (INTCONB) REGISTER. 1 = pin is compared to DEFVAL, 0 = pin is compared to previous state (default)
static constexpr uint8_t REG_IOCON = 0x0A; ///< CONFIGURATION (IOCON) REGISTER
//							   = 0x0B;			///< CONFIGURATION (IOCON) REGISTER
static constexpr uint8_t REG_GPPUA =
    0x0C; ///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
static constexpr uint8_t REG_GPPUB =
    0x0D; ///< PULL-UP RESISTOR CONFIGURATION (GPPUB) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
static constexpr uint8_t REG_INTFA = 0x0E;   ///< INTERRUPT FLAG (INTFA) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
static constexpr uint8_t REG_INTFB = 0x0F;   ///< INTERRUPT FLAG (INTFB) REGISTER, READ ONLY: 1 = This Pin Triggered the Interrupt
static constexpr uint8_t REG_INTCAPA = 0x10; ///< INTERRUPT CAPTURE (INTCAPA) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
static constexpr uint8_t REG_INTCAPB = 0x11; ///< INTERRUPT CAPTURE (INTCAPB) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
static constexpr uint8_t REG_GPIOA = 0x12;   ///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
static constexpr uint8_t REG_GPIOB = 0x13;   ///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch
static constexpr uint8_t REG_OLATA =
    0x14; ///< OUTPUT LATCH REGISTER (OLATA), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value
static constexpr uint8_t REG_OLATB =
    0x15; ///< OUTPUT LATCH REGISTER (OLATB), 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value

static constexpr uint8_t IOCON_HAEN = (1U << 3);
static constexpr uint8_t IOCON_BANK = (1U << 7);

static constexpr uint8_t DEFAULT_I2C_ADDRESS = 0x20;

static constexpr uint32_t SPI_SPEED_MAX_HZ = 10000000;    ///< 10 MHz
static constexpr uint32_t SPI_SPEED_DEFAULT_HZ = 2000000; ///< 2 MHz

static constexpr uint8_t SPI_CMD_WRITE = 0x40;
static constexpr uint8_t SPI_CMD_READ = 0x41;
} // namespace mcp23x17

#endif  // MCP23X17_H_
