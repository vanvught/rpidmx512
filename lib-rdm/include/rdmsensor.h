/**
 * @file rdmsensor.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMSENSOR_H_
#define RDMSENSOR_H_

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <algorithm>

#include "rdm_e120.h"

 #include "firmware/debug/debug_debug.h"

namespace rdm::sensor
{
struct Defintion
{
    uint8_t sensor;
    uint8_t type;
    uint8_t unit;
    uint8_t prefix;
    int16_t range_min;
    int16_t range_max;
    int16_t normal_min;
    int16_t normal_max;
    char description[32];
    uint8_t length;
    uint8_t recorded_supported;
};

struct Values
{
    int16_t present;
    int16_t lowest_detected;
    int16_t highest_detected;
    int16_t recorded;
    uint8_t sensor_requested;
};

inline constexpr int16_t RANGE_MIN = -32768;
inline constexpr int16_t RANGE_MAX = +32767;
inline constexpr int16_t NORMAL_MIN = -32768;
inline constexpr int16_t NORMAL_MAX = +32767;
inline constexpr int16_t TEMPERATURE_ABS_ZERO = -273;

inline constexpr uint8_t RECORDED_SUPPORTED = (1U << 0);
inline constexpr uint8_t LOW_HIGH_DETECT = (1U << 1);

template <class T> constexpr int16_t SafeRangeMax(const T& a)
{
    static_assert(sizeof(int16_t) <= sizeof(T), "T");
    return (a > static_cast<T>(INT16_MAX)) ? INT16_MAX : static_cast<int16_t>(a);
}

template <class T> constexpr int16_t SafeRangeMin(const T& a)
{
    static_assert(sizeof(int16_t) <= sizeof(T), "T");
    return (a < static_cast<T>(INT16_MIN)) ? INT16_MIN : static_cast<int16_t>(a);
}
} // namespace rdm::sensor

class RDMSensor
{
   public:
    explicit RDMSensor(uint8_t sensor) : sensor_(sensor)
    {
        DEBUG_ENTRY();

        sensor_defintion_.sensor = sensor_;
        sensor_defintion_.type = E120_SENS_OTHER;
        sensor_defintion_.unit = E120_UNITS_NONE;
        sensor_defintion_.prefix = E120_PREFIX_NONE;
        sensor_defintion_.range_min = rdm::sensor::RANGE_MIN;
        sensor_defintion_.range_max = rdm::sensor::RANGE_MAX;
        sensor_defintion_.normal_min = rdm::sensor::RANGE_MIN;
        sensor_defintion_.normal_max = rdm::sensor::RANGE_MAX;
        sensor_defintion_.description[0] = '\0';
        sensor_defintion_.length = 0;
        sensor_defintion_.recorded_supported = rdm::sensor::RECORDED_SUPPORTED | rdm::sensor::LOW_HIGH_DETECT;

        sensor_values_.present = 0;
        sensor_values_.lowest_detected = rdm::sensor::RANGE_MAX;
        sensor_values_.highest_detected = rdm::sensor::RANGE_MIN;
        sensor_values_.recorded = 0;
        sensor_values_.sensor_requested = sensor_;

        DEBUG_EXIT();
    }

    virtual ~RDMSensor() = default;

   public:
    void SetType(uint8_t type) { sensor_defintion_.type = type; }

    void SetUnit(uint8_t unit) { sensor_defintion_.unit = unit; }

    void SetPrefix(uint8_t prefix) { sensor_defintion_.prefix = prefix; }

    void SetRangeMin(int16_t range_min) { sensor_defintion_.range_min = range_min; }

    void SetRangeMax(int16_t range_max) { sensor_defintion_.range_max = range_max; }

    void SetNormalMin(int16_t normal_min) { sensor_defintion_.normal_min = normal_min; }

    void SetNormalMax(int16_t normal_max) { sensor_defintion_.normal_max = normal_max; }

    void SetDescription(const char* description)
    {
        DEBUG_ENTRY();

        assert(description != nullptr);
        uint32_t i;

        for (i = 0; i < 32 && description[i] != 0; i++)
        {
            sensor_defintion_.description[i] = description[i];
        }

        sensor_defintion_.length = static_cast<uint8_t>(i);

        DEBUG_EXIT();
    }

    void Print()
    {
        printf("%d [%.*s]\n", sensor_defintion_.sensor, sensor_defintion_.length, sensor_defintion_.description);
        printf(" RangeMin  %d\n", sensor_defintion_.range_min);
        printf(" RangeMax  %d\n", sensor_defintion_.range_max);
        printf(" NormalMin %d\n", sensor_defintion_.normal_min);
        printf(" NormalMax %d\n", sensor_defintion_.normal_max);
    }

    uint8_t GetSensor() const { return sensor_; }

    const struct rdm::sensor::Defintion* GetDefintion() { return &sensor_defintion_; }

    const struct rdm::sensor::Values* GetValues()
    {
        DEBUG_ENTRY();
        const auto kValue = this->GetValue();

        sensor_values_.present = kValue;
        sensor_values_.lowest_detected = std::min(sensor_values_.lowest_detected, kValue);
        sensor_values_.highest_detected = std::max(sensor_values_.highest_detected, kValue);

        DEBUG_EXIT();
        return &sensor_values_;
    }

    void SetValues()
    {
        DEBUG_ENTRY();
        const auto kValue = this->GetValue();

        sensor_values_.present = kValue;
        sensor_values_.lowest_detected = kValue;
        sensor_values_.highest_detected = kValue;
        sensor_values_.recorded = kValue;

        DEBUG_EXIT();
    }

    void Record()
    {
        DEBUG_ENTRY();
        const auto kValue = this->GetValue();

        sensor_values_.present = kValue;
        sensor_values_.recorded = kValue;
        sensor_values_.lowest_detected = std::min(sensor_values_.lowest_detected, kValue);
        sensor_values_.highest_detected = std::max(sensor_values_.highest_detected, kValue);

        DEBUG_EXIT();
    }

    virtual bool Initialize() = 0;
    virtual int16_t GetValue() = 0;

   private:
    uint8_t sensor_;
    rdm::sensor::Defintion sensor_defintion_;
    rdm::sensor::Values sensor_values_;
};

#endif  // RDMSENSOR_H_
