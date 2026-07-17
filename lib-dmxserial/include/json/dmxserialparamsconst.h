/**
 * @file dmxserialparamsconst.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_DMXSERIALPARAMSCONST_H_
#define JSON_DMXSERIALPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct DmxSerialParamsConst {
    static constexpr char kFileName[] = "dmxserial.json";

    static constexpr auto kType = json::MakeSimpleKey("type");
    // UART
    static constexpr auto kUartBaud = json::MakeSimpleKey("uart_baud");
    static constexpr auto kUartBits = json::MakeSimpleKey("uart_bits");
    static constexpr auto kUartParity = json::MakeSimpleKey("uart_parity");
    static constexpr auto kUartStopbits = json::MakeSimpleKey("uart_stopbits");
    // SPI
    static constexpr auto kSpiSpeedHz = json::MakeSimpleKey("spi_speed_hz");
    static constexpr auto kSpiMode = json::MakeSimpleKey("spi_mode");
    // I2C
    static constexpr auto kI2CAddress = json::MakeSimpleKey("i2c_address");
    static constexpr auto kI2CSpeedMode = json::MakeSimpleKey("i2c_speed_mode");
};
} // namespace json

#endif // JSON_DMXSERIALPARAMSCONST_H_
