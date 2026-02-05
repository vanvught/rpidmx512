/**
 * @file dmxnodeparams.cpp
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:infogd32-dmx.org
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

#ifdef DEBUG_DMXNODEPARAMS
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>

#include "json/dmxnodeparams.h"
#include "json/dmxnodeparamsconst.h"
#include "dmxnode_utils.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "dmxnode.h"
#include "dmxnode_nodetype.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "common/utils/utils_enum.h"
#include "firmware/debug/debug_debug.h"

using common::store::dmxnode::Flags;

namespace json
{
DmxNodeParams::DmxNodeParams()
{
    ConfigStore::Instance().Copy(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void DmxNodeParams::SetPersonality(const char* val, [[maybe_unused]] uint32_t len)
{
    store_dmxnode_.personality = common::ToValue(dmxnode::GetPersonality(val));
}

void DmxNodeParams::SetNodeName(const char* val, uint32_t len)
{
    len = len > (common::store::dmxnode::kNodeNameLength - 1) ? common::store::dmxnode::kNodeNameLength - 1 : len;
    memcpy(reinterpret_cast<char*>(store_dmxnode_.long_name), val, len);

    for (uint32_t i = len; i < common::store::dmxnode::kNodeNameLength - 1; i++)
    {
        store_dmxnode_.long_name[i] = '\0';
    }
}

void DmxNodeParams::SetFailsafe(const char* val, [[maybe_unused]] uint32_t len)
{
    store_dmxnode_.fail_safe = common::ToValue(dmxnode::GetFailsafe(val));
}

void DmxNodeParams::SetDisableMergeTimeout(const char* val, [[maybe_unused]] uint32_t len)
{
    if (len != 1) return;

    store_dmxnode_.flags = common::SetFlagValue(store_dmxnode_.flags, Flags::Flag::kDisableMergeTimeout, val[0] != '0');
}

void DmxNodeParams::SetLabelPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    val_len = val_len > (common::store::dmxnode::kLabelNameLength - 1) ? common::store::dmxnode::kLabelNameLength - 1 : val_len;
    memcpy(reinterpret_cast<char*>(store_dmxnode_.label[kIndex]), val, val_len);

    for (uint32_t i = val_len; i < common::store::dmxnode::kLabelNameLength - 1; i++)
    {
        store_dmxnode_.label[kIndex][i] = '\0';
    }
}

void DmxNodeParams::SetUniversePort(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto v = ParseValue<uint16_t>(val, val_len);
    store_dmxnode_.universe[kIndex] = v;
}

void DmxNodeParams::SetDirectionPort(const char* key, uint32_t key_len, const char* val, [[maybe_unused]] uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto direction = store_dmxnode_.direction;
    json::PortSet<dmxnode::PortDirection>(kIndex, dmxnode::GetPortDirection(val), direction);
    store_dmxnode_.direction = direction;
}

void DmxNodeParams::SetMergeModePort(const char* key, uint32_t key_len, const char* val, [[maybe_unused]] uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto merge_mode = store_dmxnode_.merge_mode;
    json::PortSet<dmxnode::MergeMode>(kIndex, dmxnode::GetMergeMode(val), merge_mode);
    store_dmxnode_.merge_mode = merge_mode;
}

void DmxNodeParams::SetOutputStylePort(const char* key, uint32_t key_len, const char* val, [[maybe_unused]] uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');
    const auto kOutputStyle = dmxnode::GetOutputStyle(val);

    if (kOutputStyle != dmxnode::OutputStyle::kDelta)
    {
        store_dmxnode_.output_style |= static_cast<uint8_t>(1U << kIndex);
    }
    else
    {
        store_dmxnode_.output_style &= static_cast<uint8_t>(~(1U << kIndex));
    }
}

void DmxNodeParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kDmxNodeKeys);
    ConfigStore::Instance().Store(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void DmxNodeParams::Set()
{
    DEBUG_ENTRY();

    auto* dmx_node = DmxNodeNodeType::Get();
    assert(dmx_node != nullptr);

    dmx_node->SetLongName(reinterpret_cast<char*>(store_dmxnode_.long_name));
    dmx_node->SetFailSafe(static_cast<dmxnode::FailSafe>(store_dmxnode_.fail_safe));
    dmx_node->SetDisableMergeTimeout(common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kDisableMergeTimeout));

    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
        {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= dmxnode::kMaxPorts)
            {
                break;
            }

            dmx_node->SetShortName(kPortIndex, reinterpret_cast<char*>(store_dmxnode_.label[config_port_index]));
            dmx_node->SetUniverse(kPortIndex, store_dmxnode_.universe[config_port_index]);
            const auto kPortDirection = json::PortGet<dmxnode::PortDirection>(config_port_index, store_dmxnode_.direction);
            dmx_node->SetDirection(kPortIndex, kPortDirection);
            const auto kPortMergeMode = json::PortGet<dmxnode::MergeMode>(config_port_index, store_dmxnode_.merge_mode);
            dmx_node->SetMergeMode(kPortIndex, kPortMergeMode);
#if defined(OUTPUT_HAVE_STYLESWITCH)
            const auto kOutputStyle = GetOutputStyleSet(1U << config_port_index);
            dmx_node->SetOutputStyle(kPortIndex, kOutputStyle);
#endif
        }
    }

#ifndef NDEBUG
    Dump();
#endif
    DEBUG_EXIT();
}

void DmxNodeParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxNodeParamsConst::kFileName);

    printf(" %s=%s [%u]\n", json::DmxNodeParamsConst::kPersonality.name, dmxnode::GetPersonality(static_cast<dmxnode::Personality>(store_dmxnode_.personality)),
           store_dmxnode_.personality);
    printf(" %s=%s\n", json::DmxNodeParamsConst::kNodeName.name, store_dmxnode_.long_name);
    printf(" %s=%s [%u]\n", json::DmxNodeParamsConst::kFailsafe.name, dmxnode::GetFailsafe(static_cast<dmxnode::FailSafe>(store_dmxnode_.fail_safe)),
           store_dmxnode_.fail_safe);
    printf(" %s=%u\n", json::DmxNodeParamsConst::kDisableMergeTimeout.name, common::IsFlagSet(store_dmxnode_.flags, Flags::Flag::kDisableMergeTimeout));

    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        for (uint32_t port_index = 0; port_index < dmxnode::kConfigPortCount; port_index++)
        {
            printf(" %s=%s\n", json::DmxNodeParamsConst::kLabelPort[port_index].name, reinterpret_cast<char*>(store_dmxnode_.label[port_index]));
            printf(" %s=%u\n", json::DmxNodeParamsConst::kUniversePort[port_index].name, store_dmxnode_.universe[port_index]);
            const auto kPortDirection = json::PortGet<dmxnode::PortDirection>(port_index, store_dmxnode_.direction);
            printf(" %s=%s\n", json::DmxNodeParamsConst::kDirectionPort[port_index].name, dmxnode::GetPortDirection(kPortDirection));
            const auto kPortMergeMode = json::PortGet<dmxnode::MergeMode>(port_index, store_dmxnode_.merge_mode);
            printf(" %s=%s\n", json::DmxNodeParamsConst::kMergeModePort[port_index].name, dmxnode::GetMergeMode(kPortMergeMode));
            const auto kOutputStyle = GetOutputStyleSet(1U << port_index);
            printf(" %s=%s\n", DmxNodeParamsConst::kOutputStylePort[port_index].name, dmxnode::GetOutputStyle(kOutputStyle));
        }
    }

    auto& dmx_node = *DmxNodeNodeType::Get();
    dmx_node.Print();
}
} // namespace json