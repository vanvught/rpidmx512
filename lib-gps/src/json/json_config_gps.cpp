/**
 * @file json_config_gps.cpp
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

#include "json/json_format_helpers.h"
#include "utc.h"
#include "json/gpsparams.h"
#include "json/json_helpers.h"
#include "json/gpsparamsconst.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "configstore.h"
#include "gps.h"

using common::store::gps::Flags;

namespace json::config
{
uint32_t GetGps(char* buffer, uint32_t length)
{
    auto& gps = *GPS::Get();

    int32_t hours = 0;
    uint32_t minutes = 0;
    hal::utc::SplitOffset(gps.GetUtcOffset(), hours, minutes);
    const auto kFlags = ConfigStore::Instance().GpsGet(&common::store::Gps::flags);

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    doc[GpsParamsConst::kModule.name] = gps::GetModule(gps.GetModule());
	    doc[GpsParamsConst::kEnable.name] = static_cast<uint32_t>(common::IsFlagSet(kFlags, Flags::Flag::kEnable));
	    char offset[format::kOffsetBufferSize];
	    doc[GpsParamsConst::kUtcOffset.name] = format::UtcOffset(hours, minutes, offset);
    });
}

void SetGps(const char* buffer, uint32_t buffer_size)
{
    ::json::GpsParams gps_params;
    gps_params.Store(buffer, buffer_size);
    gps_params.Set();
}
} // namespace json::config
