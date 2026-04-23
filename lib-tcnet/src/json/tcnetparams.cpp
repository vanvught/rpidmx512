/**
 * @file tcnetparams.cpp
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "json/tcnetparams.h"
#include "common/utils/utils_enum.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "tcnet.h"

using common::store::tcnet::Flags;

namespace json
{
TcNetParams::TcNetParams()
{
    ConfigStore::Instance().Copy(&store_tcnet, &ConfigurationStore::tcnet);
}

void TcNetParams::SetNodeName(const char* val, uint32_t len)
{
    len = len > (common::store::tcnet::kNodeNameLength - 1) ? common::store::tcnet::kNodeNameLength - 1 : len;
    memcpy(reinterpret_cast<char*>(store_tcnet.node_name), val, len);

    for (uint32_t i = len; i < common::store::tcnet::kNodeNameLength - 1; i++)
    {
        store_tcnet.node_name[i] = '\0';
    }
}

void TcNetParams::SetLayer(const char* val, uint32_t len)
{
    if (len != 1)
    {
        return;
    }

    const auto kLayer = tcnet::GetLayer(val[0]);
    store_tcnet.layer = common::ToValue(kLayer);
}

void TcNetParams::SetTimecodeType(const char* val, uint32_t len)
{
    if (len != 2)
    {
        return;
    }

    const auto kValue = json::ParseValue<uint8_t>(val, 2);

    switch (kValue)
    {
        case 24:
            store_tcnet.time_code_type = common::ToValue(tcnet::TimeCodeType::kTimecodeTypeFilm);
            break;
        case 25:
            store_tcnet.time_code_type = common::ToValue(tcnet::TimeCodeType::kTimecodeTypeEbu25Fps);
            break;
        case 29:
            store_tcnet.time_code_type = common::ToValue(tcnet::TimeCodeType::kTimecodeTypeDf);
            break;
        case 30:
            store_tcnet.time_code_type = common::ToValue(tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps);
            break;
        default:
            break;
    }
}

void TcNetParams::SetUseTimecode(const char* val, [[maybe_unused]] uint32_t len)
{
    if (len != 1) return;

    store_tcnet.flags = common::SetFlagValue(store_tcnet.flags, Flags::Flag::kUseTimecode, val[0] != '0');
}

void TcNetParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kTcNetKeys);
    ConfigStore::Instance().Store(&store_tcnet, &ConfigurationStore::tcnet);
}

void TcNetParams::Set()
{
    auto& tcnet = *TCNet::Get();
    const auto kFlags = store_tcnet.flags;

    tcnet.SetLayer(common::FromValue<tcnet::Layer>(store_tcnet.layer));
    tcnet.SetTimeCodeType(common::FromValue<tcnet::TimeCodeType>(store_tcnet.time_code_type));
    tcnet.SetUseTimeCode(common::IsFlagSet(kFlags, Flags::Flag::kUseTimecode));

#ifndef NDEBUG
    tcnet.Print();
    Dump();
#endif
}

void TcNetParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::TcNetParamsConst::kFileName);
    printf(" %s=%s\n", TcNetParamsConst::kNodeName.name, store_tcnet.node_name);
    printf(" %s=%c [%d]\n", TcNetParamsConst::kLayer.name, tcnet::GetLayer(common::FromValue<tcnet::Layer>(store_tcnet.layer)), store_tcnet.layer);
    printf(" %s=%d\n", TcNetParamsConst::kTimecodeType.name, store_tcnet.time_code_type);
}
} // namespace json
