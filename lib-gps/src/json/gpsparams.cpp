/**
 * @file gpsparams.cpp
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

#ifdef DEBUG_GPSPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "gps.h"
#include "json/gpsparams.h"
#include "json/gpsparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "common/utils/utils_enum.h"
#include "firmware/debug/debug_debug.h"

using common::store::gps::Flags;

namespace json {
GpsParams::GpsParams() {
    ConfigStore::Instance().Copy(&store_gps, &ConfigurationStore::gps);
}

void GpsParams::SetModule(const char* val, uint32_t len) {
    if (len >= gps::module::kMaxNameLength) {
        return;
    }

    char module[gps::module::kMaxNameLength];
    memcpy(module, val, len);
    module[len] = '\0';

    store_gps.module = common::ToValue(gps::GetModule(module));
}

void GpsParams::SetEnable(const char* val, [[maybe_unused]] uint32_t len) {
    if (len != 1) return;

    store_gps.flags = common::SetFlagValue(store_gps.flags, Flags::Flag::kEnable, val[0] != '0');
}

void GpsParams::SetUtcOffset(const char* val, uint32_t len) {
    int32_t hours;
    uint32_t minutes;

    if (hal::utc::ParseOffset(val, len, hours, minutes)) {
        DEBUG_PUTS("Parse OK");

        int32_t utc_offset;

        if (hal::utc::ValidateOffset(hours, minutes, utc_offset)) {
            DEBUG_PUTS("Validate OK");
            store_gps.utc_offset = utc_offset;
        }
    } else {
        DEBUG_PUTS("Parse ERROR");
    }
}

void GpsParams::Store(const char* buffer, uint32_t buffer_size) {
    ParseJsonWithTable(buffer, buffer_size, kGpsKeys);
    ConfigStore::Instance().Store(&store_gps, &ConfigurationStore::gps);

#ifndef NDEBUG
    Dump();
#endif
}

void GpsParams::Set() {
    auto& gps = *GPS::Get();

    gps.SetUtcOffset(store_gps.utc_offset);

#ifndef NDEBUG
    Dump();
#endif
}

void GpsParams::Dump() {
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::GpsParamsConst::kFileName);
    printf(" %s=%s [%u]\n", GpsParamsConst::kModule.name, gps::GetModule(static_cast<gps::Module>(store_gps.module)), store_gps.module);
    printf(" %s=%u\n", GpsParamsConst::kEnable.name, common::IsFlagSet(store_gps.flags, Flags::Flag::kEnable));
    int32_t hours;
    uint32_t minutes;
    hal::utc::SplitOffset(store_gps.utc_offset, hours, minutes);
    printf(" %s=%d:%u [%d] \n", GpsParamsConst::kUtcOffset.name, hours, minutes, store_gps.utc_offset);
}
} // namespace json