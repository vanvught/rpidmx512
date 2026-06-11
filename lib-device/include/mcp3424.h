/**
 * @file mcp3424.h
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

#ifndef MCP3424_H_
#define MCP3424_H_

#include <cstdint>

namespace adc::mcp3424 {
enum class Gain {
    kPgaX1, ///< Default
    kPgaX2,
    kPgaX4,
    kPgaX8,
};

enum class Resolution {
    kSample12Bits, ///< Default
    kSample14Bits,
    kSample16Bits,
    kSample18Bits
};

enum class Conversion {
    kOneShot,
    kContinuous ///< Default
};
} // namespace adc::mcp3424

class MCP3424 {
   public:
    explicit MCP3424(uint8_t address = 0);

    bool IsConnected() { return is_connected_; }

    void SetGain(adc::mcp3424::Gain gain);
    adc::mcp3424::Gain GetGain() const;

    void SetResolution(adc::mcp3424::Resolution resolution);
    adc::mcp3424::Resolution GetResolution() const;

    void SetConversion(adc::mcp3424::Conversion conversion);
    adc::mcp3424::Conversion GetConversion() const;

    uint32_t GetRaw(uint32_t channel);
    double GetVoltage(uint32_t channel);

   private:
    uint8_t address_{0};
    bool is_connected_{false};
    uint8_t config_;
    double lsb_;
};

#endif // MCP3424_H_
