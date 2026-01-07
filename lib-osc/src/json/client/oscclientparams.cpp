/**
 * @file oscclientparams.cpp
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

#ifdef DEBUG_OSCCLIENTPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/oscclientparams.h"
#include "json/oscclientparamsconst.h"
#include "json/oscparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "ip4/ip4_helpers.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "ip4/ip4_address.h"
#include "oscclient.h"

using common::store::osc::client::Flags;

namespace json
{
OscClientParams::OscClientParams()
{
    ConfigStore::Instance().Copy(&store_oscclient, &ConfigurationStore::osc_client);
}

void OscClientParams::SetIncomingPort(const char* val, uint32_t len)
{
    if (len <= 3)
    {
        return;
    }

    auto v = ParseValue<uint16_t>(val, len);
    store_oscclient.incoming_port = v;
}

void OscClientParams::SetOutgoingPort(const char* val, uint32_t len)
{
    if (len <= 3)
    {
        return;
    }

    auto v = ParseValue<uint16_t>(val, len);
    store_oscclient.outgoing_port = v;
}

void OscClientParams::SetServerIp(const char* val, uint32_t len)
{
    store_oscclient.server_ip = net::ParseIpString(val, len);
}

void OscClientParams::SetPingDisable(const char* val, [[maybe_unused]] uint32_t len)
{
    ParseAndApply<uint8_t>(val, len, [](uint8_t v) { store_oscclient.flags = common::SetFlagValue(store_oscclient.flags, Flags::Flag::kPingDisable, v != 0); });
}

void OscClientParams::SetPingDelay(const char* val, uint32_t len)
{
    if (len >= 3)
    {
        return;
    }

    auto v = ParseValue<uint8_t>(val, len);

    if ((v >= 2) && (v <= 60))
    {
        store_oscclient.ping_delay = v;
    }
}

void OscClientParams::SetCmd(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    if (val_len > common::store::osc::client::kCmdPathLength - 1)
    {
        return;
    }

    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - '0');
    auto* dst = store_oscclient.cmd[kIndex];

    if (val_len >= 1)
    {
        if (val[0] != '/')
        {
            dst[0] = '\0';
            return;
        }
    }

    memcpy(dst, val, val_len);
    dst[val_len] = '\0';
}

void OscClientParams::SetLed(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    if (val_len > common::store::osc::client::kLedPathLength - 1)
    {
        return;
    }

    const char kSuffix = key[key_len - 1];
    const auto kIndex = static_cast<uint8_t>(kSuffix - '0');
    auto* dst = store_oscclient.led[kIndex];

    if (val_len >= 1)
    {
        if (val[0] != '/')
        {
            dst[0] = '\0';
            return;
        }
    }

    memcpy(dst, val, val_len);
    dst[val_len] = '\0';
}

void OscClientParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kOscClientKeys);
    ConfigStore::Instance().Store(&store_oscclient, &ConfigurationStore::osc_client);

#ifndef NDEBUG
    Dump();
#endif
}

void OscClientParams::Set()
{
    auto& osc_client = OscClient::Instance();

    osc_client.SetServerIP(store_oscclient.server_ip);
    osc_client.SetPortOutgoing(store_oscclient.outgoing_port);
    osc_client.SetPortIncoming(store_oscclient.incoming_port);
    osc_client.SetPingDisable(common::IsFlagSet(store_oscclient.flags, Flags::Flag::kPingDisable));
    osc_client.SetPingDelaySeconds(store_oscclient.ping_delay);
    osc_client.CopyCmds(reinterpret_cast<const char*>(&store_oscclient.cmd));
    osc_client.CopyLeds(reinterpret_cast<const char*>(&store_oscclient.led));

#ifndef NDEBUG
    Dump();
#endif
}

void OscClientParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::OscClientParamsConst::kFileName);
    printf(" %s=" IPSTR "\n", OscClientParamsConst::kServerIp.name, IP2STR(store_oscclient.server_ip));
    printf(" %s=%u\n", OscParamsConst::kOutgoingPort, store_oscclient.outgoing_port);
    printf(" %s=%u\n", OscParamsConst::kIncomingPort, store_oscclient.incoming_port);
    printf(" %s=%u\n", OscClientParamsConst::kPingDisable, common::IsFlagSet(store_oscclient.flags, Flags::Flag::kPingDisable));
    printf(" %s=%u\n", OscClientParamsConst::kPingDelay, store_oscclient.ping_delay);

    for (uint32_t i = 0; i < common::store::osc::client::kCmdCount; i++)
    {
        printf(" %s=[%s]\n", OscClientParamsConst::kCmd[0].name, reinterpret_cast<char*>(&store_oscclient.cmd[i]));
    }

    for (uint32_t i = 0; i < common::store::osc::client::kLedCount; i++)
    {
        printf(" %s=[%s]\n", OscClientParamsConst::kLed[0].name, reinterpret_cast<char*>(&store_oscclient.led[i]));
    }
}
} // namespace json