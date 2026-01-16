/**
 * @file modeparams.cpp
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

#include "json/modeparams.h"
#include "json/modeparamsconst.h"
#include "json/dmxnodeparamsconst.h"
#include "json/sparkfundmxparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "dmxnode.h"
#include "common/utils/utils_flags.h"
#include "firmware/debug/debug_printbits.h"
 #include "firmware/debug/debug_debug.h"

using common::store::l6470dmx::mode::Flags;

namespace json
{
ModeParams::ModeParams(uint32_t motor_index) : motor_index_(motor_index)
{
    DEBUG_PRINTF("Motor index: %u", motor_index_);

    assert(motor_index < common::store::l6470dmx::kMaxMotors);
    strncpy(file_name_, SparkFunDmxParamsConst::kFileNameMotor, sizeof(file_name_));
    file_name_[5] = static_cast<char>(motor_index + '0');

    ConfigStore::Instance().DmxL6470CopyModeIndexed(motor_index, &store_mode);
}

void ModeParams::SetDmxMode(const char* val, uint32_t len)
{
    if (len == 1)
    {
        const auto kMode = json::ParseValue<uint8_t>(val, 1);
        if (kMode < L6470DMXMODE_UNDEFINED)
        {
            store_mode.dmx_mode = kMode;
        }
    }
}

void ModeParams::SetDmxStartAddress(const char* val, uint32_t len)
{
    const auto kV = ParseValue<uint16_t>(val, len);
    store_mode.dmx_start_address = kV;
}

void ModeParams::SetMaxSteps(const char* val, uint32_t len)
{
    const auto kV = ParseValue<uint32_t>(val, len);
    store_mode.max_steps = kV;
}

void ModeParams::SetSwitchAct(const char* val, uint32_t len)
{
    if (len == 4)
    {
        if (memcmp(val, "copy", 4) == 0)
        {
            store_mode.switch_action = L6470_ABSPOS_COPY;
        }
    }
}

void ModeParams::SetSwitchDir(const char* val, uint32_t len)
{
    if (len == 7)
    {
        if (memcmp(val, "forward", 7) == 0)
        {
            store_mode.switch_dir = L6470_DIR_FWD;
        }
    }
}

void ModeParams::SetSwitchSps(const char* val, uint32_t len)
{
    const auto kV = ParseValue<uint32_t>(val, len);
    store_mode.switch_steps_per_sec = kV;
}

void ModeParams::SetSwitch(const char* val, [[maybe_unused]] uint32_t len)
{
    if (len != 1) return;

    store_mode.flags = common::SetFlagValue(store_mode.flags, Flags::Flag::kUseSwitch, val[0] != '0');
}

void ModeParams::Store(const char* buffer, uint32_t buffer_size)
{
    store_mode.flags = 0;
    store_mode.dmx_mode = L6470DMXMODE_UNDEFINED;
    store_mode.dmx_start_address = dmxnode::kAddressInvalid;
    store_mode.switch_action = L6470_ABSPOS_RESET;
    store_mode.switch_dir = L6470_DIR_REV;

    ParseJsonWithTable(buffer, buffer_size, kModeKeys);
    ConfigStore::Instance().DmxL6470StoreModeIndexed(motor_index_, &store_mode);

#ifndef NDEBUG
    Dump();
#endif
}

void ModeParams::Dump()
{
	const auto kFlags = store_mode.flags;
	debug::PrintBits(kFlags);
	printf("flags=%.4x\n", kFlags);
    printf(" %s=%u\n", ModeParamsConst::kDmxMode.name, store_mode.dmx_mode);
    printf(" %s=%u\n", DmxNodeParamsConst::kDmxStartAddress.name, store_mode.dmx_start_address);

    for (uint32_t i = 0; i < common::store::l6470dmx::mode::kMaxDmxFootprint; i++)
    {
        printf(" SlotInfo\n");
        printf("  Slot:%d %2x:%4x\n", i, store_mode.slot_info[i].type, store_mode.slot_info[i].category);
    }

    printf(" %s=%u steps\n", ModeParamsConst::kMaxSteps.name, store_mode.max_steps);
    printf(" %s=%u\n", ModeParamsConst::kSwitchAct.name, store_mode.switch_action);
    printf(" %s=%u\n", ModeParamsConst::kSwitchDir.name, store_mode.switch_dir);
    printf(" %s=%u step/s\n", ModeParamsConst::kSwitchSps.name, store_mode.switch_steps_per_sec);
    printf(" %s=%u\n", ModeParamsConst::kSwitch.name, common::IsFlagSet(kFlags, common::store::l6470dmx::mode::Flags::Flag::kUseSwitch));
}
} // namespace json