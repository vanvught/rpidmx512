/**
 * @file e131params.cpp
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

#ifdef DEBUG_E131PARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/json_parsehelper.h"
#include "e131bridge.h"
#include "json/e131params.h"
#include "json/e131paramsconst.h"
#include "json/json_parser.h"
#include "dmxnode.h"
#include "configstore.h"

namespace json
{
E131Params::E131Params()
{
    ConfigStore::Instance().Copy(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void E131Params::SetPriority(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - 'a');

    auto v = ParseValue<uint8_t>(val, val_len);
    store_dmxnode_.priority[kIndex] = v;
}

void E131Params::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kE131PriorityKeys);
    ConfigStore::Instance().Store(&store_dmxnode_, &ConfigurationStore::dmx_node);
}

void E131Params::Set()
{
    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        auto& e131bridge = *E131Bridge::Get();

        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
        {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= dmxnode::kMaxPorts)
            {
                break;
            }
            e131bridge.SetPriority(kPortIndex, store_dmxnode_.priority[config_port_index]);
        }

#ifndef NDEBUG
        Dump();
#endif
    }
}

void E131Params::Dump()
{
    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::E131ParamsConst::kFileName);

        for (uint32_t port_index = 0; port_index < dmxnode::kConfigPortCount; port_index++)
        {
            printf("%s=%u\n", json::E131ParamsConst::kPriorityPort[port_index].name, store_dmxnode_.priority[port_index]);
        }
    }

    auto& e131bridge = *E131Bridge::Get();
    e131bridge.Print();
}
} // namespace json
