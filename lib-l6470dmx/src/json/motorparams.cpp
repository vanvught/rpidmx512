/**
 * @file motorparams.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#undef NDEBUG

#include <cstdint>

#include "l6470dmxmodes.h"
#include "json/motorparams.h"
#include "json/motorparamsconst.h"
#include "json/sparkfundmxparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
 #include "firmware/debug/debug_debug.h"

namespace json
{

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static constexpr float kTickS = 0.00000025f; ///< 250ns

MotorParams::MotorParams(uint32_t motor_index) : motor_index_(motor_index)
{
    DEBUG_PRINTF("Motor index: %u", motor_index_);

    assert(motor_index < common::store::l6470dmx::kMaxMotors);
    strncpy(file_name_, SparkFunDmxParamsConst::kFileNameMotor, sizeof(file_name_));
    file_name_[5] = static_cast<char>(motor_index + '0');

    ConfigStore::Instance().DmxL6470CopyMotorIndexed(motor_index, &store_motor);
}

void MotorParams::SetStepAngel(const char* val, uint32_t len)
{
    auto v = (json::Atof(val, len));
    store_motor.step_angel = v;
}

void MotorParams::SetVoltage(const char* val, uint32_t len)
{
    auto v = (json::Atof(val, len));
    store_motor.step_angel = v;
}

void MotorParams::SetCurrent(const char* val, uint32_t len)
{
    auto v = (json::Atof(val, len));
    store_motor.step_angel = v;
}

void MotorParams::SetResistance(const char* val, uint32_t len)
{
    auto v = (json::Atof(val, len));
    store_motor.step_angel = v;
}

void MotorParams::SetInductance(const char* val, uint32_t len)
{
    auto v = (json::Atof(val, len));
    store_motor.step_angel = v;
}

void MotorParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kMotorKeys);
    ConfigStore::Instance().DmxL6470StoreMotorIndexed(motor_index_, &store_motor);

#ifndef NDEBUG
    Dump();
#endif
}

static float CalcIntersectSpeed(float resistance, float inductance)
{
    if (inductance == 0) return 0;

    return (4.0f * resistance) / (2.0f * M_PI * inductance * 0.001f);
}

static uint32_t CalcIntersectSpeedReg(float f)
{
    return static_cast<uint32_t>(f * (kTickS * (1U << 26)));
}

void MotorParams::Set(L6470* l6470)
{
    assert(l6470 != nullptr);

    const auto kF = CalcIntersectSpeed(store_motor.resistance, store_motor.inductance);

    if (kF != 0)
    {
        l6470->setParam(L6470_PARAM_INT_SPD, CalcIntersectSpeedReg(kF));
    }
}

void MotorParams::Dump()
{
    printf(" %s=%.1f degree\n", MotorParamsConst::kStepAngel.name, store_motor.step_angel);
    printf(" %s=%.2f V\n", MotorParamsConst::kVoltage.name, store_motor.voltage);
    printf(" %s=%.1f A/phase\n", MotorParamsConst::kCurrent, store_motor.current);
    printf(" %s=%.1f Ohm/phase\n", MotorParamsConst::kResistance, store_motor.resistance);
    printf(" %s=%.1f mH/phase\n", MotorParamsConst::kInductance, store_motor.inductance);

    const auto kF = CalcIntersectSpeed(store_motor.resistance, store_motor.inductance);

    printf(" Intersect speed = %f step/s (register:INT_SPEED=0x%.4X)\n", kF, static_cast<unsigned int>(CalcIntersectSpeedReg(kF)));
}
} // namespace json