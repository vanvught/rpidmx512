/**
 * @file dmxmonitorparams.cpp
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

#include <cstring>

#include "common/utils/utils_enum.h"
#include "dmxmonitor.h"
#include "configstore.h"
#include "json/dmxmonitorparams.h"
#include "json/dmxmonitorparamsconst.h"
#include "json/json_parsehelper.h"
#include "json/json_parser.h"

namespace json
{
DmxMonitorParams::DmxMonitorParams()
{
    ConfigStore::Instance().Copy(&store_dmxmonitor, &ConfigurationStore::dmx_monitor);
}

void DmxMonitorParams::SetDmxStartAddress(const char* val, uint32_t len)
{
    auto v = ParseValue<uint16_t>(val, len);
    store_dmxmonitor.dmx_start_address = v;
}

void DmxMonitorParams::SetDmxMaxChannels(const char* val, uint32_t len)
{
    auto v = ParseValue<uint16_t>(val, len);
    store_dmxmonitor.dmx_max_channels = v;
}

void DmxMonitorParams::SetFormat(const char* val, uint32_t len)
{
	if (len != 3) return;
	
    if (memcmp(val, "pct", 3) == 0)
    {
        store_dmxmonitor.format = common::ToValue(dmxmonitor::Format::kPct);
    }
    else if (memcmp(val, "dec", 3) == 0)
    {
        store_dmxmonitor.format = common::ToValue(dmxmonitor::Format::kDec);
    }
    else
    {
        store_dmxmonitor.format = common::ToValue(dmxmonitor::Format::kHex);
    }
}

void DmxMonitorParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kDmxMonitorKeys);
    ConfigStore::Instance().Store(&store_dmxmonitor, &ConfigurationStore::dmx_monitor);
}

void DmxMonitorParams::Set()
{
    auto& dmxmonitor = DmxMonitor::Instance();

	dmxmonitor.SetDmxStartAddress(store_dmxmonitor.dmx_start_address);
	dmxmonitor.SetFormat(static_cast<dmxmonitor::Format>(store_dmxmonitor.format));
#if defined(__linux__) || defined(__APPLE__)
	dmxmonitor.SetMaxDmxChannels(store_dmxmonitor.dmx_max_channels);
#endif

#ifndef NDEBUG
    Dump();
#endif
}

void DmxMonitorParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxMonitorParamsConst::kFileName);

 	printf(" %s=%u\n", DmxMonitorParamsConst::kDmxStartAddress.name, store_dmxmonitor.dmx_start_address);
    printf(" %s=%u\n", DmxMonitorParamsConst::kDmxMaxChannels.name, store_dmxmonitor.dmx_max_channels);
    printf(" %s=%s [%u]\n", DmxMonitorParamsConst::kFormat.name, store_dmxmonitor.format == common::ToValue(dmxmonitor::Format::kPct) ? "pct" : (store_dmxmonitor.format == static_cast<uint8_t>(dmxmonitor::Format::kDec) ? "dec" : "hex"), static_cast<int>(store_dmxmonitor.format));
}
} // namespace json