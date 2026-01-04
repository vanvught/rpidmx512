/**
 * @file remoteconfigparams.cpp
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

#include "json/remoteconfigparams.h"
#include "json/remoteconfigparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"
#include "remoteconfig.h"

namespace json
{
RemoteConfigParams::RemoteConfigParams()
{
    ConfigStore::Instance().Copy(&store_remoteconfig, &ConfigurationStore::remote_config);
}

void RemoteConfigParams::SetDisplayName(const char* val, uint32_t len)
{
    len = len > (common::store::remoteconfig::kDisplayNameLength - 1) ? common::store::remoteconfig::kDisplayNameLength - 1 : len;
    memcpy(reinterpret_cast<char*>(store_remoteconfig.display_name), val, len);

    for (uint32_t i = len; i < common::store::remoteconfig::kDisplayNameLength; i++)
    {
        store_remoteconfig.display_name[i] = '\0';
    }
}

void RemoteConfigParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kRemoteConfigKeys);
    ConfigStore::Instance().Store(&store_remoteconfig, &ConfigurationStore::remote_config);

#ifndef NDEBUG
    Dump();
#endif
}

void RemoteConfigParams::Set()
{
    auto& remoteconfig = *RemoteConfig::Get();
    
    remoteconfig.SetDisplayName(reinterpret_cast<char *>(store_remoteconfig.display_name));

#ifndef NDEBUG
    Dump();
#endif
}

void RemoteConfigParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::RemoteConfigParamsConst::kFileName);
    printf(" %s=%s\n", RemoteConfigParamsConst::kDisplayName.name, store_remoteconfig.display_name);
}
} // namespace json
