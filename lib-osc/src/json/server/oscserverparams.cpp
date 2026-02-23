/**
 * @file oscserverparams.cpp
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

#ifdef DEBUG_OSCSERVERPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/oscserverparams.h"
#include "json/oscserverparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"

using common::store::osc::server::Flags;

namespace json
{
OscServerParams::OscServerParams()
{
    ConfigStore::Instance().Copy(&store_oscserver, &ConfigurationStore::osc_server);
}

void OscServerParams::SetIncomingPort(const char* val, uint32_t len)
{
    if (len <= 3)
    {
        return;
    }

    auto v = ParseValue<uint16_t>(val, len);
    store_oscserver.incoming_port = v;
}

void OscServerParams::SetOutgoingPort(const char* val, uint32_t len)
{
    if (len <= 3)
    {
        return;
    }

    auto v = ParseValue<uint16_t>(val, len);
    store_oscserver.outgoing_port = v;
}

void OscServerParams::SetPath(const char* val, uint32_t len)
{
    if (len > common::store::osc::server::kPathLength - 1)
    {
        return;
    }

    auto* dst = store_oscserver.path;

    if (len >= 1)
    {
        if (val[0] != '/')
        {
            dst[0] = '\0';
            return;
        }
    }

    memcpy(dst, val, len);
    dst[len] = '\0';
}

void OscServerParams::SetPathInfo(const char* val, uint32_t len)
{
    if (len > common::store::osc::server::kPathLength - 1)
    {
        return;
    }

    auto* dst = store_oscserver.path_info;

    if (len >= 1)
    {
        if (val[0] != '/')
        {
            dst[0] = '\0';
            return;
        }
    }

    memcpy(dst, val, len);
    dst[len] = '\0';
}

void OscServerParams::SetPathBlackout(const char* val, [[maybe_unused]] uint32_t len)
{
    if (len > common::store::osc::server::kPathLength - 1)
    {
        return;
    }

    auto* dst = store_oscserver.path_blackout;

    if (len >= 1)
    {
        if (val[0] != '/')
        {
            dst[0] = '\0';
            return;
        }
    }

    memcpy(dst, val, len);
    dst[len] = '\0';
}

void OscServerParams::SetTransmission(const char* val, [[maybe_unused]] uint32_t len)
{
    if (len != 1) return;

    store_oscserver.flags = common::SetFlagValue(store_oscserver.flags, Flags::Flag::kPartialTransmission, val[0] != '0');
}

void OscServerParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kOscServerKeys);
    ConfigStore::Instance().Store(&store_oscserver, &ConfigurationStore::osc_server);

#ifndef NDEBUG
    Dump();
#endif
}

void OscServerParams::Set()
{
#ifndef NDEBUG
    Dump();
#endif
}

void OscServerParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::OscServerParamsConst::kFileName);
    printf(" %s=%u\n", OscParamsConst::kIncomingPort.name, store_oscserver.incoming_port);
    printf(" %s=%u\n", OscParamsConst::kOutgoingPort.name, store_oscserver.outgoing_port);
    printf(" %s=%s\n", OscServerParamsConst::kPath.name, store_oscserver.path);
    printf(" %s=%s\n", OscServerParamsConst::kPathInfo.name, store_oscserver.path_info);
    printf(" %s=%s\n", OscServerParamsConst::kPathBlackout.name, store_oscserver.path_blackout);
    printf(" %s=%d\n", OscServerParamsConst::kTransmission.name, common::IsFlagSet(store_oscserver.flags, Flags::Flag::kPartialTransmission));
}
} // namespace json