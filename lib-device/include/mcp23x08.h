/**
 * @file mcp23x08.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MCP23X08_H_
#define MCP23X08_H_

#include <cstdint>

namespace mcp23x08
{
static constexpr uint8_t REG_IODIR = 0x00;
static constexpr uint8_t REG_IPOL = 0x01;
static constexpr uint8_t REG_GPINTEN = 0x02;
static constexpr uint8_t REG_DEFVAL = 0x03;
static constexpr uint8_t REG_INTCON = 0x04;
static constexpr uint8_t REG_IOCON = 0x05;
static constexpr uint8_t REG_GPPU = 0x06;
static constexpr uint8_t REG_INTF = 0x07;
static constexpr uint8_t REG_INTCAP = 0x08;
static constexpr uint8_t REG_GPIO = 0x09;
static constexpr uint8_t REG_OLAT = 0x0A;
} // namespace mcp23x08

#endif  // MCP23X08_H_
