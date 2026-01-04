/**
 * @file json_config_dmxmonitor.cpp
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

#include "dmxmonitor.h"
#include "json/dmxmonitorparamsconst.h"
#include "json/json_helpers.h"
#include "json/dmxmonitorparams.h"

namespace json::config
{
uint32_t GetDmxMonitor(char* buffer, uint32_t length)
{
    auto& dmxmonitor = DmxMonitor::Instance();
    const auto kFormat = dmxmonitor.GetFormat();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
    	doc[json::DmxMonitorParamsConst::kDmxStartAddress.name] = dmxmonitor.GetDmxStartAddress();
    	doc[json::DmxMonitorParamsConst::kDmxMaxChannels.name] = dmxmonitor.GetMaxDmxChannels();
    	doc[json::DmxMonitorParamsConst::kFormat.name] = (kFormat == dmxmonitor::Format::kPct ? "pct" : (kFormat == dmxmonitor::Format::kDec) ? "dec" : "hex");
    });
}

void SetDmxMonitor(const char* buffer, uint32_t buffer_size)
{
    ::json::DmxMonitorParams dmxmonitor_params;
    dmxmonitor_params.Store(buffer, buffer_size);
    dmxmonitor_params.Set();
}
} // namespace json::config