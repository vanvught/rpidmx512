/**
 * @file rdmdeviceparams.cpp
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

#if defined(DEBUG_RDMDEVICEPARAMS)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>

#include "rdmdevice.h"
#include "json/rdmdeviceparams.h"
#include "json/rdmdeviceparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"

namespace json
{
RdmDeviceParams::RdmDeviceParams()
{
    ConfigStore::Instance().Copy(&store_rdmdevice, &ConfigurationStore::rdm_device);
}

void RdmDeviceParams::SetLabel(const char* val, uint32_t len)
{
    memcpy(store_rdmdevice.device_root_label, val, std::max(len, static_cast<uint32_t>(RDM_DEVICE_LABEL_MAX_LENGTH)));
    store_rdmdevice.device_root_label_length = static_cast<uint8_t>(len);
}

void RdmDeviceParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kRdmDeviceKeys);
    ConfigStore::Instance().Store(&store_rdmdevice, &ConfigurationStore::rdm_device);

#ifndef NDEBUG
    Dump();
#endif
}

void RdmDeviceParams::Set()
{
    DEBUG_ENTRY();
    auto& rdmdevice = rdm::device::Device::Instance();

    struct rdm::device::InfoData info_data = {.data = reinterpret_cast<char*>(store_rdmdevice.device_root_label),
                                              .length = store_rdmdevice.device_root_label_length};

    rdmdevice.SetLabel(&info_data);

#ifndef NDEBUG
    Dump();
#endif
    DEBUG_EXIT();
}

void RdmDeviceParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RdmDeviceParamsConst::kFileName);
    printf(" %s=%.*s\n", RdmDeviceParamsConst::kLabel.name, store_rdmdevice.device_root_label_length, store_rdmdevice.device_root_label);
}
} // namespace json