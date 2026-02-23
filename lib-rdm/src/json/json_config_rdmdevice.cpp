/**
 * @file json_config_rdmdevice.cpp
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

#include "json/rdmdeviceparams.h"
#include "json/json_helpers.h"
#include "json/rdmdeviceparamsconst.h"
#include "common/utils/utils_hex.h"
#include "rdmdevice.h"

namespace json::config
{
uint32_t GetRdmDevice(char* buffer, uint32_t length)
{
    auto& rdmdevice = RdmDevice::Get();

    struct rdm::DeviceInfoData info_data;
    rdmdevice.GetLabel(&info_data);
    char label[RDM_DEVICE_LABEL_MAX_LENGTH + 1];
    memcpy(label, info_data.data, info_data.length);
    label[info_data.length] = '\0';

    return json::helpers::Serialize(
        buffer, length,
        [&](JsonDoc& doc)
        {
            doc[json::RdmDeviceParamsConst::kLabel.name] = label;
            char product[5];
            doc[json::RdmDeviceParamsConst::kProductCategory.name] = common::hex::ToStringLower<4>(product, rdmdevice.GetProductCategory());
            doc[json::RdmDeviceParamsConst::kProductDetail.name] = common::hex::ToStringLower<4>(product, rdmdevice.GetProductDetail());
        });
}

void SetRdmDevice(const char* buffer, uint32_t buffer_size)
{
    ::json::RdmDeviceParams rdmdevice_params;
    rdmdevice_params.Store(buffer, buffer_size);
    rdmdevice_params.Set();
}
} // namespace json::config