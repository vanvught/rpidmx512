/**
 * @file l6470params.cpp
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
#include <cassert>

#include "l6470dmxmodes.h"
#include "json/l6470paramsconst.h"
#include "json/l6470params.h"
#include "json/sparkfundmxparamsconst.h"
#include "json/json_parser.h"
#include "common/utils/utils_flags.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"

using common::store::l6470dmx::l6470::Flags;

namespace json
{
template <typename EnumFlag> inline uint32_t SetFlag(const char* val, uint32_t len, uint32_t flags, EnumFlag flag_bit)
{
    if ((len == 0) || !isdigit(static_cast<int>(val[0])))
    {
        return common::SetFlagValue(flags, flag_bit, false);
    }

    return common::SetFlagValue(flags, flag_bit, true);
}

L6470Params::L6470Params(uint32_t motor_index) : motor_index_(motor_index)
{
    DEBUG_PRINTF("L6470 index: %u", motor_index_);

    assert(motor_index < common::store::l6470dmx::kMaxMotors);
    strncpy(file_name_, SparkFunDmxParamsConst::kFileNameMotor, sizeof(file_name_));
    file_name_[5] = static_cast<char>(motor_index + '0');

    ConfigStore::Instance().DmxL6470CopyL6470Indexed(motor_index, &store_l6470);
}

void L6470Params::SetMinSpeed(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetMinSpeed);
    store_l6470.min_speed = ParseValue<uint32_t>(val, len);
}

void L6470Params::SetMaxSpeed(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetMaxSpeed);
    store_l6470.max_speed = ParseValue<uint32_t>(val, len);
}

void L6470Params::SetAcc(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetDec);
    store_l6470.acc = ParseValue<uint32_t>(val, len);
}

void L6470Params::SetDec(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetDec);
    store_l6470.dec = ParseValue<uint32_t>(val, len);
}

void L6470Params::SetKvalHold(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetKvalHold);
    store_l6470.kval_hold = ParseValue<uint8_t>(val, len);
}

void L6470Params::SetKvalRun(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetKvalRun);
    store_l6470.kval_run = ParseValue<uint8_t>(val, len);
}

void L6470Params::SetKvalAcc(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetKvalAcc);
    store_l6470.kval_acc = ParseValue<uint8_t>(val, len);
}

void L6470Params::SetKvalDec(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetKvalDec);
    store_l6470.kval_dec = ParseValue<uint8_t>(val, len);
}

void L6470Params::SetMicroSteps(const char* val, uint32_t len)
{
    store_l6470.flags = SetFlag(val, len, store_l6470.flags, Flags::Flag::kIsSetMicroSteps);
    store_l6470.micro_steps = ParseValue<uint8_t>(val, len);
}

void L6470Params::Store(const char* buffer, uint32_t buffer_size)
{
    store_l6470.flags = 0;

    ParseJsonWithTable(buffer, buffer_size, kL6470Keys);
    ConfigStore::Instance().DmxL6470StoreL6470Indexed(motor_index_, &store_l6470);

#ifndef NDEBUG
    Dump();
#endif
}

void L6470Params::Set(L6470* l6470)
{
    assert(l6470 != nullptr);

    const auto kFlags = store_l6470.flags;

    if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetMinSpeed))
    {
        l6470->setMinSpeed(store_l6470.min_speed);
    }

    if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetMaxSpeed))
    {
        l6470->setMinSpeed(store_l6470.max_speed);
    }

    if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetAcc))
    {
        l6470->setAcc(store_l6470.acc);
    }

    if (common::IsFlagSet(kFlags, Flags::Flag::kIsSetDec))
    {
        l6470->setDec(store_l6470.dec);
    }
}

void L6470Params::Dump()
{
    printf("flags=%.4x\n", store_l6470.flags);
    printf(" %s=%u\n", L6470ParamsConst::kMinSpeed.name, store_l6470.min_speed);
    printf(" %s=%u\n", L6470ParamsConst::kMaxSpeed.name, store_l6470.max_speed);
    printf(" %s=%u\n", L6470ParamsConst::kAcc.name, store_l6470.acc);
    printf(" %s=%u\n", L6470ParamsConst::kDec.name, store_l6470.dec);
    printf(" %s=%u\n", L6470ParamsConst::kKvalHold.name, store_l6470.kval_hold);
    printf(" %s=%u\n", L6470ParamsConst::kKvalRun.name, store_l6470.kval_run);
    printf(" %s=%u\n", L6470ParamsConst::kKvalAcc.name, store_l6470.kval_acc);
    printf(" %s=%u\n", L6470ParamsConst::kKvalDec.name, store_l6470.kval_dec);
    printf(" %s=%u\n", L6470ParamsConst::kMicroSteps.name, store_l6470.micro_steps);
}
} // namespace json