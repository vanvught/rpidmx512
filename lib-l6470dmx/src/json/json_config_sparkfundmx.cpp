/**
 * @file json_config_sparkfundmx.cpp
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

#include <cstdint>

#include "json/json_helpers.h"
#include "json/sparkfundmxparams.h"
#include "json/modeparams.h"
#include "json/modeparamsconst.h"
#include "json/l6470params.h"
#include "json/dmxnodeparamsconst.h"
#include "json/motorparams.h"
#include "json/motorparamsconst.h"
#include "json/l6470paramsconst.h"
#include "json/json_jsondoc.h"
#include "configurationstore.h"
#include "configstore.h"
#include "common/utils/utils_flags.h"
#include "json/sparkfundmxparamsconst.h"

using common::store::l6470dmx::sparkfun::Flags;

namespace json::config
{
static constexpr const char kNotSet[] = "not set";

static uint32_t Get(char* buffer, uint32_t length, uint32_t motor_index)
{
    common::store::l6470dmx::SparkFun store_sparkfun;

    if (motor_index >= common::store::l6470dmx::kMaxMotors)
    {
        ConfigStore::Instance().DmxL6470CopySparkFunGlobal(&store_sparkfun);
    }
    else
    {
        ConfigStore::Instance().DmxL6470CopySparkFunIndexed(motor_index, &store_sparkfun);
    }

    const auto kSparkfunFlags = store_sparkfun.flags;

    return json::helpers::Serialize(buffer, length,
                                    [&](JsonDoc& doc)
                                    {
                                        if (common::IsFlagSet(kSparkfunFlags, Flags::Flag::kIsSetPosition))
                                        {
                                            doc[SparkFunDmxParamsConst::kPosition.name] = store_sparkfun.position;
                                        }
                                        else
                                        {
                                            doc[SparkFunDmxParamsConst::kPosition.name] = kNotSet;
                                        }

                                        if (common::IsFlagSet(kSparkfunFlags, Flags::Flag::kIsSetSpiCs))
                                        {
                                            doc[SparkFunDmxParamsConst::kSpiCs.name] = store_sparkfun.spi_cs;
                                        }
                                        else
                                        {
                                            doc[SparkFunDmxParamsConst::kSpiCs.name] = kNotSet;
                                        }

                                        if (common::IsFlagSet(kSparkfunFlags, Flags::Flag::kIsSetResetPin))
                                        {
                                            doc[SparkFunDmxParamsConst::kResetPin.name] = store_sparkfun.reset_pin;
                                        }
                                        else
                                        {
                                            doc[SparkFunDmxParamsConst::kResetPin.name] = kNotSet;
                                        }

                                        if (common::IsFlagSet(kSparkfunFlags, Flags::Flag::kIsSetBusyPin))
                                        {
                                            doc[SparkFunDmxParamsConst::kBusyPin.name] = store_sparkfun.busy_pin;
                                        }
                                        else
                                        {
                                            doc[SparkFunDmxParamsConst::kBusyPin.name] = kNotSet;
                                        }

                                        if (motor_index < common::store::l6470dmx::kMaxMotors)
                                        {
                                            common::store::l6470dmx::Mode mode;
                                            ConfigStore::Instance().DmxL6470CopyModeIndexed(motor_index, &mode);

                                            const auto kFlags = mode.flags;

                                            const auto kMode = mode.dmx_mode;

                                            if (kMode < L6470DMXMODE_UNDEFINED)
                                            {
                                                doc[ModeParamsConst::kDmxMode.name] = kMode;
                                            }
                                            else
                                            {
                                                doc[ModeParamsConst::kDmxMode.name] = kNotSet;
                                            }

                                            doc[DmxNodeParamsConst::kDmxStartAddress.name] = mode.dmx_start_address;
                                            // DMX Slot info
                                            doc[ModeParamsConst::kMaxSteps.name] = mode.max_steps;
                                            doc[ModeParamsConst::kSwitchAct.name] = json::dmxmode::GetSwitchAction(mode.switch_action);
                                            doc[ModeParamsConst::kSwitchDir.name] = json::dmxmode::GetSwitchDir(mode.switch_dir);
                                            doc[ModeParamsConst::kSwitchSps.name] = mode.switch_steps_per_sec;
                                            doc[ModeParamsConst::kSwitch.name] =
                                                common::IsFlagSet(kFlags, common::store::l6470dmx::mode::Flags::Flag::kUseSwitch);

                                            common::store::l6470dmx::Motor motor;
                                            ConfigStore::Instance().DmxL6470CopyMotorIndexed(motor_index, &motor);

                                            char f[8];

                                            snprintf(f, sizeof(f) - 1, "%.1f", motor.step_angel);
                                            doc[MotorParamsConst::kStepAngel.name] = f;
                                            snprintf(f, sizeof(f) - 1, "%.2f", motor.voltage);
                                            doc[MotorParamsConst::kVoltage.name] = f;
                                            snprintf(f, sizeof(f) - 1, "%.1f", motor.current);
                                            doc[MotorParamsConst::kCurrent.name] = f;
                                            snprintf(f, sizeof(f) - 1, "%.1f", motor.resistance);
                                            doc[MotorParamsConst::kResistance.name] = f;
                                            snprintf(f, sizeof(f) - 1, "%.1f", motor.inductance);
                                            doc[MotorParamsConst::kInductance.name] = f;

                                            {
                                                common::store::l6470dmx::L6470 l6470;
                                                ConfigStore::Instance().DmxL6470CopyL6470Indexed(motor_index, &l6470);

                                                using common::store::l6470dmx::l6470::Flags;
                                                const auto kL6470Flags = l6470.flags;

                                                if (common::IsFlagSet(kL6470Flags, Flags::Flag::kIsSetMinSpeed))
                                                {
                                                    doc[L6470ParamsConst::kMinSpeed.name] = l6470.min_speed;
                                                }
                                                else
                                                {
                                                    doc[L6470ParamsConst::kMinSpeed.name] = kNotSet;
                                                }

                                                if (common::IsFlagSet(kL6470Flags, Flags::Flag::kIsSetMaxSpeed))
                                                {
                                                    doc[L6470ParamsConst::kMaxSpeed.name] = l6470.max_speed;
                                                }
                                                else
                                                {
                                                    doc[L6470ParamsConst::kMaxSpeed.name] = kNotSet;
                                                }

                                                doc[L6470ParamsConst::kAcc.name] = l6470.acc;
                                                doc[L6470ParamsConst::kDec.name] = l6470.dec;
                                                doc[L6470ParamsConst::kKvalHold.name] = l6470.kval_hold;
                                                doc[L6470ParamsConst::kKvalRun.name] = l6470.kval_run;
                                                doc[L6470ParamsConst::kKvalAcc.name] = l6470.kval_acc;
                                                doc[L6470ParamsConst::kKvalDec.name] = l6470.kval_dec;
                                                doc[L6470ParamsConst::kMicroSteps.name] = l6470.micro_steps;
                                            }
                                        }
                                    });
}

uint32_t GetSparkFunDmx(char* buffer, uint32_t length)
{
    return Get(buffer, length, common::store::l6470dmx::kMaxMotors);
}

uint32_t GetDmxL6470Motor0(char* buffer, uint32_t length)
{
    return Get(buffer, length, 0);
}

uint32_t GetDmxL6470Motor1(char* buffer, uint32_t length)
{
    return Get(buffer, length, 1);
}

uint32_t GetDmxL6470Motor2(char* buffer, uint32_t length)
{
    return Get(buffer, length, 2);
}

uint32_t GetDmxL6470Motor3(char* buffer, uint32_t length)
{
    return Get(buffer, length, 3);
}

uint32_t GetDmxL6470Motor4(char* buffer, uint32_t length)
{
    return Get(buffer, length, 4);
}

uint32_t GetDmxL6470Motor5(char* buffer, uint32_t length)
{
    return Get(buffer, length, 5);
}

uint32_t GetDmxL6470Motor6(char* buffer, uint32_t length)
{
    return Get(buffer, length, 6);
}

uint32_t GetDmxL6470Motor7(char* buffer, uint32_t length)
{
    return Get(buffer, length, 7);
}

void SetSparkFunDmx(const char* buffer, uint32_t buffer_size)
{
    ::json::SparkFunDmxParams sparkfundmx_params;
    sparkfundmx_params.Store(buffer, buffer_size);
}

template <uint32_t M> void SetSetDmxL6470Motor(const char* buffer, uint32_t buffer_size)
{
    static_assert(M < common::store::l6470dmx::kMaxMotors);

    ::json::SparkFunDmxParams sparkfundmx_params(M);
    sparkfundmx_params.Store(buffer, buffer_size);

    ::json::ModeParams mode_params(M);
    mode_params.Store(buffer, buffer_size);

    ::json::MotorParams motor_params(M);
    motor_params.Store(buffer, buffer_size);

    ::json::L6470Params l6470_params(M);
    l6470_params.Store(buffer, buffer_size);
}

void SetDmxL6470Motor0(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<0>(buffer, buffer_size);
}

void SetDmxL6470Motor1(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<1>(buffer, buffer_size);
}

void SetDmxL6470Motor2(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<2>(buffer, buffer_size);
}

void SetDmxL6470Motor3(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<3>(buffer, buffer_size);
}

void SetDmxL6470Motor4(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<4>(buffer, buffer_size);
}

void SetDmxL6470Motor5(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<5>(buffer, buffer_size);
}

void SetDmxL6470Motor6(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<6>(buffer, buffer_size);
}

void SetDmxL6470Motor7(const char* buffer, uint32_t buffer_size)
{
    SetSetDmxL6470Motor<7>(buffer, buffer_size);
}
} // namespace json::config
