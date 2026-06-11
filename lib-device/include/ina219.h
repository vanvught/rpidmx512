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

namespace sensor {
namespace ina219 {
inline constexpr uint16_t RANGE_16V = 0x0000; ///< 0-16V Range
inline constexpr uint16_t RANGE_32V = 0x2000; ///< 0-32V Range

inline constexpr uint16_t GAIN_40MV = 0x0000;  ///< Gain 1, 40mV Range
inline constexpr uint16_t GAIN_80MV = 0x0800;  ///< Gain 2, 80mV Range
inline constexpr uint16_t GAIN_160MV = 0x1000; ///< Gain 4, 160mV Range
inline constexpr uint16_t GAIN_320MV = 0x1800; ///< Gain 8, 320mV Range

inline constexpr uint16_t BUS_RES_9BIT = 0x0080;  ///< 9-bit bus res = 0..511
inline constexpr uint16_t BUS_RES_10BIT = 0x0100; ///< 10-bit bus res = 0..1023
inline constexpr uint16_t BUS_RES_11BIT = 0x0200; ///< 11-bit bus res = 0..2047
inline constexpr uint16_t BUS_RES_12BIT = 0x0400; ///< 12-bit bus res = 0..4097

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
inline constexpr uint16_t SHUNT_RES_12BIT_128S = 0x0078; ///< 128 x 12-bit shunt samples averaged together

inline constexpr uint16_t MODE_POWER_DOWN = 0x0000;
inline constexpr uint16_t MODE_SHUNT_TRIG = 0x0001;
inline constexpr uint16_t MODE_BUS_TRIG = 0x0002;
inline constexpr uint16_t MODE_SHUNT_BUS_TRIG = 0x0003;
inline constexpr uint16_t MODE_ADC_OFF = 0x0004;
inline constexpr uint16_t MODE_SHUNT_CONT = 0x0005;
inline constexpr uint16_t MODE_BUS_CONT = 0x0006;
inline constexpr uint16_t MODE_SHUNT_BUS_CONT = 0x0007;

struct Config {
    uint16_t range = RANGE_32V;
    uint16_t gain = GAIN_320MV;
    uint16_t bus_res = BUS_RES_12BIT;
    uint16_t shunt_res = SHUNT_RES_12BIT_1S;
    uint16_t mode = MODE_SHUNT_BUS_CONT;
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

class INA219 {
   public:
    explicit INA219(uint8_t address = 0);

    void Configure(ina219::Config& config);
    void Calibrate(float r_shunt_value = 0.1f, float i_max_expected = 2.0f);

    bool Initialize() { return initialized_; }

    float GetShuntCurrent();
    float GetBusVoltage();
    float GetBusPower();

   private:
    int16_t GetBusVoltageRaw();

   private:
    uint8_t address_{0};
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
