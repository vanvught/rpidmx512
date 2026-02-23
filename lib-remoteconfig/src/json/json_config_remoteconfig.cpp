/**
 * @file json_config_remoteconfig.cpp
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

#ifdef DEBUG_REMOTECONFIGCONFIG
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "json/json_jsondoc.h"
#include "json/remoteconfigparams.h"
#include "json/remoteconfigparamsconst.h"
#include "json/json_jsondoc.h"
#include "configstore.h"

namespace json::config
{
uint32_t GetRemoteConfig(char* buffer, uint32_t length)
{
    assert(buffer != nullptr);
    assert(length != 0);

    uint8_t display_name[common::store::remoteconfig::kDisplayNameLength];
    ConfigStore::Instance().RemoteConfigCopyArray(display_name, &common::store::RemoteConfig::display_name);
    display_name[common::store::remoteconfig::kDisplayNameLength - 1] = '\0'; // Just to be safe

    JsonDoc doc(buffer, length);

    doc[RemoteConfigParamsConst::kDisplayName.name] = reinterpret_cast<char *>(display_name);

    doc.End();

    return doc.Size();
}

void SetRemoteConfig(const char* buffer, uint32_t buffer_size)
{
    ::json::RemoteConfigParams remoteconfig_params;
    remoteconfig_params.Store(buffer, buffer_size);
    remoteconfig_params.Set();
}
} // namespace json::config