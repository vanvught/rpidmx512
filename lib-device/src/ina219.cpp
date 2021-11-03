/**
 * @file ina219.h
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "ina219.h"

#include "hal_i2c.h"

#include "debug.h"

static float CEILING_POS(float f) {
	const auto i = static_cast<int>(f);
	if (f == static_cast<float>(i)) {
		return static_cast<float>(i);
	}
	return static_cast<float>(i + 1);
}

namespace sensor {
namespace ina219 {
static constexpr uint8_t I2C_ADDRESS = 0x40;
namespace reg {
static constexpr uint8_t CONFIG = 0x00;
// static constexpr uint8_t SHUNTVOLTAGE = 0x01;
static constexpr uint8_t BUSVOLTAGE = 0x02;
static constexpr uint8_t POWER = 0x03;
static constexpr uint8_t CURRENT = 0x04;
static constexpr uint8_t CALIBRATION = 0x05;
namespace value {
static constexpr auto READ_DELAY_US	= 800;
}  // namespace value
}  // namespace reg
}  // namespace ina219

using namespace sensor::ina219;

INA219::INA219(uint8_t nAddress) : HAL_I2C(nAddress == 0  ? I2C_ADDRESS : nAddress) {
	m_bIsInitialized = HAL_I2C::IsConnected();

	if (m_bIsInitialized) {
		Config config;
		Configure(config);
		Calibrate();
	}
}

void INA219::Configure(Config& config) {

	switch (config.range) {
	case RANGE_32V:
		m_Info.v_bus_max = 32.0f;
		break;
	case RANGE_16V:
		m_Info.v_bus_max = 16.0f;
		break;
	}

	switch (config.gain) {
	case GAIN_320MV:
		m_Info.v_shunt_max = 0.32f;
		break;
	case GAIN_160MV:
		m_Info.v_shunt_max = 0.16f;
		break;
	case GAIN_80MV:
		m_Info.v_shunt_max = 0.08f;
		break;
	case GAIN_40MV:
		m_Info.v_shunt_max = 0.04f;
		break;
	}

	const uint16_t nConfig = config.range | config.gain| config.bus_res | config.shunt_res | config.mode;

	DEBUG_PRINTF("nConfig=%x", nConfig);

	HAL_I2C::WriteRegister(reg::CONFIG, nConfig);
}

void INA219::Calibrate(float r_shunt_value, float i_max_expected) {
	const float minimum_lsb = i_max_expected / 32767;

	m_Info.r_shunt = r_shunt_value;

	m_Info.current_lsb = (static_cast<uint16_t>(minimum_lsb * 100000000));
	m_Info.current_lsb /= 100000000;
	m_Info.current_lsb /= 0.0001f;
	m_Info.current_lsb = CEILING_POS(m_Info.current_lsb);
	m_Info.current_lsb *= 0.0001f;

	m_Info.power_lsb = m_Info.current_lsb * 20;

	const auto nCalibrationValue = static_cast<uint16_t>((0.04096f / (m_Info.current_lsb * m_Info.r_shunt)));

	DEBUG_PRINTF("nCalibrationValue=%x", nCalibrationValue);

	HAL_I2C::WriteRegister(reg::CALIBRATION, nCalibrationValue);
}

float INA219::GetShuntCurrent() {
	const float fValue = HAL_I2C::ReadRegister16DelayUs(reg::CURRENT, reg::value::READ_DELAY_US) * m_Info.current_lsb;
	return fValue;
}

int16_t INA219::GetBusVoltageRaw() {
	auto voltage = HAL_I2C::ReadRegister16DelayUs(reg::BUSVOLTAGE, reg::value::READ_DELAY_US);
	voltage = static_cast<uint16_t>(voltage >> 3);

	return static_cast<int16_t>(voltage * 4);
}

float INA219::GetBusVoltage() {
	return GetBusVoltageRaw() * 0.001f;
}

float INA219::GetBusPower() {
	const float fValue = HAL_I2C::ReadRegister16DelayUs(reg::POWER, reg::value::READ_DELAY_US) * m_Info.power_lsb;
	return fValue;
}

}  // namespace sensor
