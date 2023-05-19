/**
 * @file mcp9808.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "mcp9808.h"

#include "hal_i2c.h"

namespace sensor {
namespace mcp9808 {
static constexpr uint8_t I2C_ADDRESS = 0x18;
namespace reg {
// static constexpr uint8_t UPPER_TEMP = 0x02;
// static constexpr uint8_t LOWER_TEMP = 0x03;
// static constexpr uint8_t CRIT_TEMP = 0x04;
static constexpr uint8_t AMBIENT_TEMP = 0x05;
static constexpr uint8_t MANUF_ID = 0x06;
static constexpr uint8_t DEVICE_ID = 0x07;
}  // namespace reg
}  // namespace mcp9808

using namespace sensor::mcp9808;

MCP9808::MCP9808(uint8_t nAddress) : HAL_I2C(nAddress == 0 ? I2C_ADDRESS : nAddress) {
	m_bIsInitialized = IsConnected();

	if (m_bIsInitialized) {
		m_bIsInitialized = (ReadRegister16(reg::MANUF_ID) == 0x0054);
	}

	if (m_bIsInitialized) {
		m_bIsInitialized = (ReadRegister16(reg::DEVICE_ID) == 0x0400);
	}
}

float MCP9808::Get() {
	const auto nValue = ReadRegister16(reg::AMBIENT_TEMP);
	auto fTemperature = static_cast<float>(nValue & 0x0FFF);

	fTemperature /= 16.0f;

	if ((nValue & 0x1000) == 0x1000) {
		fTemperature -= 256.0f;
	}

	return fTemperature;
}

}  // namespace sensor
