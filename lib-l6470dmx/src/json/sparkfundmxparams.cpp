/**
 * @file sparkfundmxparams.cpp
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

#ifdef DEBUG_SPARKFUNPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/sparkfundmxparams.h"
#include "json/sparkfundmxparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"
#include "sparkfundmx.h"
#include "json/json_parsehelper.h"
#include "common/utils/utils_flags.h"
 #include "firmware/debug/debug_debug.h"

using common::store::l6470dmx::sparkfun::Flags;

namespace json
{
SparkFunDmxParams::SparkFunDmxParams()
{
    motor_index_ = common::store::l6470dmx::kMaxMotors;
    ConfigStore::Instance().DmxL6470CopySparkFunGlobal(&store_sparkfun);
}

SparkFunDmxParams::SparkFunDmxParams(uint32_t motor_index) : motor_index_(motor_index)
{
    DEBUG_PRINTF("Motor index: %u", motor_index_);

    assert(motor_index < common::store::l6470dmx::kMaxMotors);
    strncpy(file_name_, SparkFunDmxParamsConst::kFileNameMotor, sizeof(file_name_));
    file_name_[5] = static_cast<char>(motor_index + '0');

    ConfigStore::Instance().DmxL6470CopySparkFunIndexed(motor_index, &store_sparkfun);
}

void SparkFunDmxParams::SetPosition(const char* val, uint32_t len)
{
    if (len == 1)
    {
        const auto kCh = val[0];

        if (kCh >= '0' && kCh <= '9')
        {
            const auto kPosition = static_cast<uint8_t>(kCh - '0');
            if (kPosition < common::store::l6470dmx::kMaxMotors)
            {
                store_sparkfun.position = kPosition;
                store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetPosition, true);
                return;
            }
        }
    }

    store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetPosition, false);
}

void SparkFunDmxParams::SetSpiCs(const char* val, uint32_t len)
{
    if ((len == 0) || (len > 3))
    {
        store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetSpiCs, false);
        return;
    }

    store_sparkfun.spi_cs = json::ParseValue<uint8_t>(val, len);
    store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetSpiCs, true);
}

void SparkFunDmxParams::SetResetPin(const char* val, uint32_t len)
{
    if ((len == 0) || (len > 3))
    {
        store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetResetPin, false);
        return;
    }

    store_sparkfun.reset_pin = json::ParseValue<uint8_t>(val, len);
    store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetResetPin, true);
}

void SparkFunDmxParams::SetBusyPin(const char* val, uint32_t len)
{
    if ((len == 0) || (len > 3))
    {
        store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetBusyPin, false);
        return;
    }

    store_sparkfun.busy_pin = json::ParseValue<uint8_t>(val, len);
    store_sparkfun.flags = common::SetFlagValue(store_sparkfun.flags, Flags::Flag::kIsSetBusyPin, true);
}

void SparkFunDmxParams::Store(const char* buffer, uint32_t buffer_size)
{
    store_sparkfun.flags = 0;

    ParseJsonWithTable(buffer, buffer_size, kSparkFunKeys);
    
    if (motor_index_ >= common::store::l6470dmx::kMaxMotors)
    {
        ConfigStore::Instance().DmxL6470StoreSparkFunGlobal(&store_sparkfun);
    }
    else
    {
        ConfigStore::Instance().DmxL6470StoreSparkFunIndexed(motor_index_, &store_sparkfun);
    }
#ifndef NDEBUG
    Dump();
#endif
}

void SparkFunDmxParams::Set(SparkFunDmx* sparkfundmx)
{
    assert(sparkfundmx != nullptr);
    const auto kFlags = store_sparkfun.flags;

    if (motor_index_ >= common::store::l6470dmx::kMaxMotors)
    {
        DEBUG_PUTS("Set Global");
        // #if !defined(H3)
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetSpiCs))
        {
            sparkfundmx->SetGlobalSpiCs(store_sparkfun.spi_cs);
        }
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetResetPin))
        {
            sparkfundmx->SetGlobalResetPin(store_sparkfun.reset_pin);
        }
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetBusyPin))
        {
            sparkfundmx->SetGlobalBusyPin(store_sparkfun.busy_pin);
        }
        // #endif
    }
    else
    {
        DEBUG_PUTS("Set Local");
        // #if !defined(H3)
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetSpiCs))
        {
            sparkfundmx->SetLocalSpiCs(store_sparkfun.spi_cs);
        }
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetResetPin))
        {
            sparkfundmx->SetLocalResetPin(store_sparkfun.reset_pin);
        }
        if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetBusyPin))
        {
            sparkfundmx->SetLocalBusyPin(store_sparkfun.busy_pin);
        }
        // #endif
    }

#ifndef NDEBUG
    Dump();
#endif
}

void SparkFunDmxParams::Dump()
{
    if (motor_index_ >= common::store::l6470dmx::kMaxMotors)
    {
        printf("%s::%s \'%s\' (global settings):\n", __FILE__, __FUNCTION__, SparkFunDmxParamsConst::kFileName);
    }
    else
    {
        printf("%s::%s \'%s\' :\n", __FILE__, __FUNCTION__, file_name_);
    }

    const auto kFlags = store_sparkfun.flags;

    printf(" %s=%u [%d]\n", SparkFunDmxParamsConst::kPosition.name, store_sparkfun.position,
           common::IsFlagSet(kFlags, Flags::Flag::kIsSetPosition));

    // #if !defined(H3)

    printf(" %s=%u [%d]\n", SparkFunDmxParamsConst::kSpiCs.name, store_sparkfun.spi_cs,
           common::IsFlagSet(kFlags, Flags::Flag::kIsSetSpiCs));

    // #endif

    printf(" %s=%u [%d]\n", SparkFunDmxParamsConst::kResetPin.name, store_sparkfun.reset_pin,
           common::IsFlagSet(kFlags, Flags::Flag::kIsSetResetPin));
    printf(" %s=%u [%d]\n", SparkFunDmxParamsConst::kBusyPin.name, store_sparkfun.busy_pin,
           common::IsFlagSet(kFlags, Flags::Flag::kIsSetBusyPin));
}
} // namespace json