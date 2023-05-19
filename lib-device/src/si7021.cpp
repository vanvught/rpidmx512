/**
 * @file si7021.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "si7021.h"

#include "hal_i2c.h"

namespace sensor {
namespace si7021 {
static constexpr uint8_t I2C_ADDRESS = 0x40;
namespace reg {
// static constexpr uint8_t TRIGGER_TEMP_MEASURE_HOLD = 0xE3;
// static constexpr uint8_t TRIGGER_HUMD_MEASURE_HOLD = 0xE5;
static constexpr uint8_t TRIGGER_TEMP_MEASURE_NOHOLD = 0xF3;
static constexpr uint8_t TRIGGER_HUMD_MEASURE_NOHOLD = 0xF5;
// static constexpr uint8_t WRITE_USER_REG = 0xE6;
// static constexpr uint8_t READ_USER_REG = 0xE7;
// static constexpr uint8_t SOFT_RESET = 0xFE;
}  // namespace reg
}  // namespace si7021

using namespace sensor::si7021;

SI7021::SI7021(uint8_t nAddress): HAL_I2C(nAddress == 0  ? I2C_ADDRESS : nAddress) {
	m_bIsInitialized = HAL_I2C::IsConnected();
}

float SI7021::GetTemperature() {
	const auto temp = static_cast<float>(ReadRaw(reg::TRIGGER_TEMP_MEASURE_NOHOLD)) / 65536.0f;
	return -46.85f + (175.72f * temp);
}

float SI7021::GetHumidity() {
	const auto humd = static_cast<float>(ReadRaw(reg::TRIGGER_HUMD_MEASURE_NOHOLD)) / 65536.0f;
	return -6.0f + (125.0f * humd);
}


uint16_t SI7021::ReadRaw(uint8_t nCmd) {
	HAL_I2C::Write(nCmd);

	char buf[3] = {0};

	for (uint32_t i = 0; i < 8; ++i) {
		udelay(10000);
		HAL_I2C::Read(buf, 3);

		if ((buf[0] & 0x3) == 2) {
			break;
		}
	}

	const auto nRawValue = static_cast<uint16_t>((buf[0] << 8) | buf[1]);

	return nRawValue & 0xFFFC;
}

}  // namespace sensor
