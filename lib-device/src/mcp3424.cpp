/**
 * @file mcp3424.cpp
 *
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

// TODO Implement Gain for GetVoltage

#include <cstdint>
#include <cassert>

#include "mcp3424.h"
#include "i2c.h"
#include "firmware/debug/debug_debug.h"

namespace adc::mcp3424 {
static constexpr uint8_t kI2CAddress = 0x68;

static constexpr uint8_t GAIN(Gain gain) {
    return static_cast<uint8_t>(gain) & 0x03;
}

static constexpr uint8_t RESOLUTION(Resolution resolution) {
    return (static_cast<uint8_t>(resolution) & 0x03) << 2;
}

static constexpr uint8_t CONVERSION(Conversion conversion) {
    return (static_cast<uint8_t>(conversion) & 0x01) << 4;
}

static constexpr uint8_t CHANNEL(uint32_t channel) {
    return (channel & 0x03) << 5;
}
} // namespace adc::mcp3424

MCP3424::MCP3424(uint8_t address) : I2c(address == 0 ? adc::mcp3424::kI2CAddress : address) {
    DEBUG_ENTRY();
    DEBUG_PRINTF("address=%x", address);
    is_connected_ = IsConnected();

    if (is_connected_) {
        SetGain(adc::mcp3424::Gain::kPgaX1);
        SetResolution(adc::mcp3424::Resolution::kSample12Bits);
        SetConversion(adc::mcp3424::Conversion::kContinuous);
    }

    DEBUG_PRINTF("is_connected_=%u", is_connected_);
    DEBUG_EXIT();
}

void MCP3424::SetGain(adc::mcp3424::Gain gain) {
    config_ &= static_cast<uint8_t>(~0x03);
    config_ |= adc::mcp3424::GAIN(gain);
}
adc::mcp3424::Gain MCP3424::GetGain() const {
    return static_cast<adc::mcp3424::Gain>(config_ & 0x3);
}

void MCP3424::SetResolution(adc::mcp3424::Resolution resolution) {
    config_ &= static_cast<uint8_t>(~((0x03) << 2));
    config_ |= adc::mcp3424::RESOLUTION(resolution);

    switch (resolution) {
        case adc::mcp3424::Resolution::kSample12Bits:
            lsb_ = 0.0005;
            break;
        case adc::mcp3424::Resolution::kSample14Bits:
            lsb_ = 0.000125;
            break;
        case adc::mcp3424::Resolution::kSample16Bits:
            lsb_ = 0.00003125;
            break;
        case adc::mcp3424::Resolution::kSample18Bits:
            lsb_ = 0.0000078125;
            break;
        default:
            [[unlikely]] assert(0);
            __builtin_unreachable();
            break;
    }
}
adc::mcp3424::Resolution MCP3424::GetResolution() const {
    return static_cast<adc::mcp3424::Resolution>((config_ >> 2) & 0x03);
}

void MCP3424::SetConversion(adc::mcp3424::Conversion conversion) {
    config_ &= static_cast<uint8_t>(~((0x01) << 4));
    config_ |= adc::mcp3424::CONVERSION(conversion);
}

adc::mcp3424::Conversion MCP3424::GetConversion() const {
    return static_cast<adc::mcp3424::Conversion>((config_ >> 4) & 0x01);
}

uint32_t MCP3424::GetRaw(uint32_t channel) {
    config_ &= static_cast<uint8_t>(~((0x03) << 5));
    config_ |= adc::mcp3424::CHANNEL(channel);

    uint32_t bytes = 3;

    if ((config_ & RESOLUTION(adc::mcp3424::Resolution::kSample18Bits)) == RESOLUTION(adc::mcp3424::Resolution::kSample18Bits)) {
        bytes = 4;
    }

    char buffer[4] = {0, 0, 0, 0};
    int32_t timeout = 8000;
	
	Setup();

    while (true) {
        Write(config_, false);
        Read(buffer, bytes, false);

        if (bytes == 4) {
            if ((buffer[3] >> 7) == 0) {
                break;
            }
        } else {
            if ((buffer[2] >> 7) == 0) {
                break;
            }
        }

        if (timeout-- == 0) {
            return UINT32_MAX;
        }
    }

    switch (static_cast<adc::mcp3424::Resolution>((config_ >> 2) & 0x03)) {
        case adc::mcp3424::Resolution::kSample12Bits:
            return static_cast<uint32_t>(((buffer[0] & 0x0f) << 8) | buffer[1]);
            break;
        case adc::mcp3424::Resolution::kSample14Bits:
            return static_cast<uint32_t>(((buffer[0] & 0x3f) << 8) | buffer[1]);
            break;
        case adc::mcp3424::Resolution::kSample16Bits:
            return static_cast<uint32_t>((buffer[0] << 8) | buffer[1]);
            break;
        case adc::mcp3424::Resolution::kSample18Bits:
            return static_cast<uint32_t>(((buffer[0] & 0x03) << 16) | (buffer[1] << 8) | buffer[2]);
            break;
        default:
            [[unlikely]] assert(0);
            __builtin_unreachable();
            break;
    }

    assert(0);
    __builtin_unreachable();
    return UINT32_MAX;
}

double MCP3424::GetVoltage(uint32_t channel) {
    const auto kVout = static_cast<double>(GetRaw(channel)) * 2 * lsb_;
    return kVout;
}
