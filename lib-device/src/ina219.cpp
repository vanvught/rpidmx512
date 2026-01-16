/**
 * @file ina219.cpp
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

 #include "firmware/debug/debug_debug.h"

static float CeilingPos(float f)
{
    const auto kI = static_cast<int>(f);
    if (f == static_cast<float>(kI))
    {
        return static_cast<float>(kI);
    }
    return static_cast<float>(kI + 1);
}

namespace sensor
{
namespace ina219
{
static constexpr uint8_t kI2CAddress = 0x40;
namespace reg
{
static constexpr uint8_t kConfig = 0x00;
// static constexpr uint8_t SHUNTVOLTAGE = 0x01;
static constexpr uint8_t kBusvoltage = 0x02;
static constexpr uint8_t kPower = 0x03;
static constexpr uint8_t kCurrent = 0x04;
static constexpr uint8_t kCalibration = 0x05;
namespace value
{
static constexpr auto kReadDelayUs = 800;
} // namespace value
} // namespace reg
} // namespace ina219

INA219::INA219(uint8_t address) : HAL_I2C(address == 0 ? sensor::ina219::kI2CAddress : address)
{
    m_bIsInitialized = HAL_I2C::IsConnected();

    if (m_bIsInitialized)
    {
        sensor::ina219::Config config;
        Configure(config);
        Calibrate();
    }
}

void INA219::Configure(sensor::ina219::Config& config)
{
    switch (config.range)
    {
        case sensor::ina219::RANGE_32V:
            m_Info.v_bus_max = 32.0f;
            break;
        case sensor::ina219::RANGE_16V:
            m_Info.v_bus_max = 16.0f;
            break;
    }

    switch (config.gain)
    {
        case sensor::ina219::GAIN_320MV:
            m_Info.v_shunt_max = 0.32f;
            break;
        case sensor::ina219::GAIN_160MV:
            m_Info.v_shunt_max = 0.16f;
            break;
        case sensor::ina219::GAIN_80MV:
            m_Info.v_shunt_max = 0.08f;
            break;
        case sensor::ina219::GAIN_40MV:
            m_Info.v_shunt_max = 0.04f;
            break;
    }

    const uint16_t kConfig = config.range | config.gain | config.bus_res | config.shunt_res | config.mode;

    DEBUG_PRINTF("kConfig=%x", kConfig);

    HAL_I2C::WriteRegister(sensor::ina219::reg::kConfig, kConfig);
}

void INA219::Calibrate(float r_shunt_value, float i_max_expected)
{
    const float kMinimumLsb = i_max_expected / 32767;

    m_Info.r_shunt = r_shunt_value;

    m_Info.current_lsb = (static_cast<uint16_t>(kMinimumLsb * 100000000));
    m_Info.current_lsb /= 100000000;
    m_Info.current_lsb /= 0.0001f;
    m_Info.current_lsb = CeilingPos(m_Info.current_lsb);
    m_Info.current_lsb *= 0.0001f;

    m_Info.power_lsb = m_Info.current_lsb * 20;

    const auto kCalibrationValue = static_cast<uint16_t>((0.04096f / (m_Info.current_lsb * m_Info.r_shunt)));

    DEBUG_PRINTF("kCalibrationValue=%x", kCalibrationValue);

    HAL_I2C::WriteRegister(sensor::ina219::reg::kCalibration, kCalibrationValue);
}

float INA219::GetShuntCurrent()
{
    const float kValue = HAL_I2C::ReadRegister16DelayUs(sensor::ina219::reg::kCurrent, sensor::ina219::reg::value::kReadDelayUs) * m_Info.current_lsb;
    return kValue;
}

int16_t INA219::GetBusVoltageRaw()
{
    auto voltage = HAL_I2C::ReadRegister16DelayUs(sensor::ina219::reg::kBusvoltage, sensor::ina219::reg::value::kReadDelayUs);
    voltage = static_cast<uint16_t>(voltage >> 3);

    return static_cast<int16_t>(voltage * 4);
}

float INA219::GetBusVoltage()
{
    return GetBusVoltageRaw() * 0.001f;
}

float INA219::GetBusPower()
{
    const float kValue = HAL_I2C::ReadRegister16DelayUs(sensor::ina219::reg::kPower, sensor::ina219::reg::value::kReadDelayUs) * m_Info.power_lsb;
    return kValue;
}

} // namespace sensor
