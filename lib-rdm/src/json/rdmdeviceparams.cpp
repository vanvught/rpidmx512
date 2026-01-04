/**
 * @file rdmdeviceparams.cpp
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
#include "common/utils/utils_hex.h"

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

void RdmDeviceParams::SetProductCategory([[maybe_unused]] const char* val, uint32_t len)
{
    if (len != 4) return;

    char h[5];
    h[4] = '\0';
    memcpy(h, val, 4);
    store_rdmdevice.product_category = static_cast<uint16_t>(common::hex::FromHex(h));
}

void RdmDeviceParams::SetProductDetail([[maybe_unused]] const char* val, uint32_t len)
{
    if (len != 4) return;

    char h[5];
    h[4] = '\0';
    memcpy(h, val, 4);
    store_rdmdevice.product_detail = static_cast<uint16_t>(common::hex::FromHex(h));
}

void RdmDeviceParams::Store(const char* buffer, uint32_t buffer_size)
{
    store_rdmdevice.product_category = E120_PRODUCT_CATEGORY_NOT_DECLARED;
    store_rdmdevice.product_detail = E120_PRODUCT_DETAIL_NOT_DECLARED;

    ParseJsonWithTable(buffer, buffer_size, kRdmDeviceKeys);
    ConfigStore::Instance().Store(&store_rdmdevice, &ConfigurationStore::rdm_device);

#ifndef NDEBUG
    Dump();
#endif
}

void RdmDeviceParams::Set()
{
    DEBUG_ENTRY();
    auto& rdmdevice = RdmDevice::Get();

    struct rdm::DeviceInfoData info_data = 
    {
		.data = reinterpret_cast<char*>(store_rdmdevice.device_root_label),                                           
		.length = store_rdmdevice.device_root_label_length
	};
	
    rdmdevice.SetLabel(&info_data);

    if (store_rdmdevice.product_category != E120_PRODUCT_CATEGORY_NOT_DECLARED) rdmdevice.SetProductCategory(store_rdmdevice.product_category);
    if (store_rdmdevice.product_detail != E120_PRODUCT_DETAIL_NOT_DECLARED) rdmdevice.SetProductDetail(store_rdmdevice.product_detail);

#ifndef NDEBUG
    Dump();
#endif
    DEBUG_EXIT();
}

void RdmDeviceParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RdmDeviceParamsConst::kFileName);
    printf(" %s=%.*s\n", RdmDeviceParamsConst::kLabel.name, store_rdmdevice.device_root_label_length, store_rdmdevice.device_root_label);
 	printf(" %s=%.4x\n", RdmDeviceParamsConst::kProductCategory.name, store_rdmdevice.product_category);
    printf(" %s=%.4x\n", RdmDeviceParamsConst::kProductDetail.name, store_rdmdevice.product_detail);
}
} // namespace json