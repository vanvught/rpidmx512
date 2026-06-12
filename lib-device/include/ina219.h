/**
 * @file ina219.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef INA219_H_
#define INA219_H_

#include <cstdint>

#include "i2c.h"
#include "firmware/debug/debug_debug.h"

namespace sensor {
namespace ina219 {
inline constexpr uint8_t kI2CAddress = 0x40;
namespace reg {
inline constexpr uint8_t kConfig = 0x00;
// static constexpr uint8_t SHUNTVOLTAGE = 0x01;
inline constexpr uint8_t kBusvoltage = 0x02;
inline constexpr uint8_t kPower = 0x03;
inline constexpr uint8_t kCurrent = 0x04;
inline constexpr uint8_t kCalibration = 0x05;
namespace value {
inline constexpr auto kReadDelayUs = 800;
} // namespace value
} // namespace reg
inline constexpr uint16_t kRange16V = 0x0000; ///< 0-16V Range
inline constexpr uint16_t kRange32V = 0x2000; ///< 0-32V Range

inline constexpr uint16_t GAIN_40MV = 0x0000;  ///< Gain 1, 40mV Range
inline constexpr uint16_t GAIN_80MV = 0x0800;  ///< Gain 2, 80mV Range
inline constexpr uint16_t GAIN_160MV = 0x1000; ///< Gain 4, 160mV Range
inline constexpr uint16_t GAIN_320MV = 0x1800; ///< Gain 8, 320mV Range

inline constexpr uint16_t kBusRes9Bit = 0x0080;  ///< 9-bit bus res = 0..511
inline constexpr uint16_t kBusRes10Bit = 0x0100; ///< 10-bit bus res = 0..1023
inline constexpr uint16_t kBusRes11Bit = 0x0200; ///< 11-bit bus res = 0..2047
inline constexpr uint16_t kBusRes12Bit = 0x0400; ///< 12-bit bus res = 0..4097

inline constexpr uint16_t SHUNT_RES_9BIT_1S = 0x0000;    ///< 1 x 9-bit shunt sample
inline constexpr uint16_t SHUNT_RES_10BIT_1S = 0x0008;   ///< 1 x 10-bit shunt sample
inline constexpr uint16_t SHUNT_RES_11BIT_1S = 0x0010;   ///< 1 x 11-bit shunt sample
inline constexpr uint16_t SHUNT_RES_12BIT_1S = 0x0018;   ///< 1 x 12-bit shunt sample
inline constexpr uint16_t SHUNT_RES_12BIT_2S = 0x0048;   ///< 2 x 12-bit shunt samples averaged together
inline constexpr uint16_t SHUNT_RES_12BIT_4S = 0x0050;   ///< 4 x 12-bit shunt samples averaged together
inline constexpr uint16_t SHUNT_RES_12BIT_8S = 0x0058;   ///< 8 x 12-bit shunt samples averaged together
inline constexpr uint16_t SHUNT_RES_12BIT_16S = 0x0060;  ///< 16 x 12-bit shunt samples averaged together
inline constexpr uint16_t SHUNT_RES_12BIT_32S = 0x0068;  ///< 32 x 12-bit shunt samples averaged together
inline constexpr uint16_t SHUNT_RES_12BIT_64S = 0x0070;  ///< 64 x 12-bit shunt samples averaged together
inline constexpr uint16_t kShuntRes12Bit128S = 0x0078; ///< 128 x 12-bit shunt samples averaged together

inline constexpr uint16_t kModePowerDown = 0x0000;
inline constexpr uint16_t kModeShuntTrig = 0x0001;
inline constexpr uint16_t kModeBusTrig = 0x0002;
inline constexpr uint16_t kModeShuntBusTrig = 0x0003;
inline constexpr uint16_t kModeAdcOff = 0x0004;
inline constexpr uint16_t kModeShuntCont = 0x0005;
inline constexpr uint16_t kModeBusCont = 0x0006;
inline constexpr uint16_t kModeShuntBusCont = 0x0007;

struct Config {
    uint16_t range = kRange32V;
    uint16_t gain = GAIN_320MV;
    uint16_t bus_res = kBusRes12Bit;
    uint16_t shunt_res = SHUNT_RES_12BIT_1S;
    uint16_t mode = kModeShuntBusCont;
};

namespace current {
inline constexpr char kDescription[] = "Current";
inline constexpr int16_t kRangeMin = -2000; // mA
inline constexpr int16_t kRangeMax = 2000;  // mA
} // namespace current
namespace voltage {
inline constexpr char kDescription[] = "Voltage";
inline constexpr int16_t kRangeMin = -32000; // mV
inline constexpr int16_t kRangeMax = 32000;  // mV
} // namespace voltage
namespace power {
inline constexpr char kDescription[] = "Power";
inline constexpr int16_t kRangeMin = -64; // W
inline constexpr int16_t kRangeMax = 64;  // W
} // namespace power
} // namespace ina219

class INA219 : I2c {
   public:
    explicit INA219(uint8_t address) : I2c(address == 0 ? sensor::ina219::kI2CAddress : address) {
        initialized_ = IsConnected();

        if (initialized_) {
            sensor::ina219::Config config;
            Configure(config);
            Calibrate(0.1f, 2.0f);
        }
    }

    void Configure(sensor::ina219::Config& config) {
        switch (config.range) {
            case sensor::ina219::kRange32V:
                info_.v_bus_max = 32.0f;
                break;
            case sensor::ina219::kRange16V:
                info_.v_bus_max = 16.0f;
                break;
        }

        switch (config.gain) {
            case sensor::ina219::GAIN_320MV:
                info_.v_shunt_max = 0.32f;
                break;
            case sensor::ina219::GAIN_160MV:
                info_.v_shunt_max = 0.16f;
                break;
            case sensor::ina219::GAIN_80MV:
                info_.v_shunt_max = 0.08f;
                break;
            case sensor::ina219::GAIN_40MV:
                info_.v_shunt_max = 0.04f;
                break;
        }

        const uint16_t kConfig = config.range | config.gain | config.bus_res | config.shunt_res | config.mode;

        DEBUG_PRINTF("kConfig=%x", kConfig);

        i2c::WriteReg(sensor::ina219::reg::kConfig, kConfig);
    }

    void Calibrate(float r_shunt_value, float i_max_expected) {
        const float kMinimumLsb = i_max_expected / 32767;

        info_.r_shunt = r_shunt_value;

        info_.current_lsb = (static_cast<uint16_t>(kMinimumLsb * 100000000));
        info_.current_lsb /= 100000000;
        info_.current_lsb /= 0.0001f;
        info_.current_lsb = CeilingPos(info_.current_lsb);
        info_.current_lsb *= 0.0001f;

        info_.power_lsb = info_.current_lsb * 20;

        const auto kCalibrationValue = static_cast<uint16_t>((0.04096f / (info_.current_lsb * info_.r_shunt)));

        DEBUG_PRINTF("kCalibrationValue=%x", kCalibrationValue);

        i2c::WriteReg(sensor::ina219::reg::kCalibration, kCalibrationValue);
    }

    float GetShuntCurrent() {
        const float kValue = ReadRegister16DelayUs(sensor::ina219::reg::kCurrent, sensor::ina219::reg::value::kReadDelayUs) * info_.current_lsb;
        return kValue;
    }

    float GetBusVoltage() { return GetBusVoltageRaw() * 0.001f; }

    float GetBusPower() {
        const float kValue = ReadRegister16DelayUs(sensor::ina219::reg::kPower, sensor::ina219::reg::value::kReadDelayUs) * info_.power_lsb;
        return kValue;
    }
	
	bool Initialize() { return initialized_; }

   private:
    int16_t GetBusVoltageRaw() {
        auto voltage = ReadRegister16DelayUs(sensor::ina219::reg::kBusvoltage, sensor::ina219::reg::value::kReadDelayUs);
        voltage = static_cast<uint16_t>(voltage >> 3);

        return static_cast<int16_t>(voltage * 4);
    }

    float CeilingPos(float f) {
        const auto kI = static_cast<int>(f);
        if (f == static_cast<float>(kI)) {
            return static_cast<float>(kI);
        }
        return static_cast<float>(kI + 1);
    }

   private:
     bool initialized_{false};

    struct Info {
        float current_lsb;
        float power_lsb;
        float v_shunt_max;
        float v_bus_max;
        float r_shunt;
    } info_;
};
} // namespace sensor

#endif // INA219_H_
