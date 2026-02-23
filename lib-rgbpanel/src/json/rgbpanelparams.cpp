/**
 * @file rgbpanelparams.cpp
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

#ifdef DEBUG_RGBPANELPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "common/utils/utils_enum.h"
#include "rgbpanel.h"
#include "json/rgbpanelparams.h"
#include "json/rgbpanelparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"

namespace json
{
RgbPanelParams::RgbPanelParams()
{
    ConfigStore::Instance().Copy(&store_rgbpanel, &ConfigurationStore::rgb_panel);
}

void RgbPanelParams::SetCols(const char* val, uint32_t len)
{
    store_rgbpanel.cols = static_cast<uint8_t>(rgbpanel::ValidateColumns(json::ParseValue<uint32_t>(val, len)));
}

void RgbPanelParams::SetRows(const char* val, uint32_t len)
{
    store_rgbpanel.rows = static_cast<uint8_t>(rgbpanel::ValidateRows(json::ParseValue<uint32_t>(val, len)));
}

void RgbPanelParams::SetChain(const char* val, uint32_t len)
{
    store_rgbpanel.chain = json::ParseValue<uint8_t>(val, len);
}

void RgbPanelParams::SetType(const char* val, [[maybe_unused]] uint32_t len)
{
    store_rgbpanel.type = common::ToValue(rgbpanel::GetType(val));
}

void RgbPanelParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kRgbPanelKeys);
    ConfigStore::Instance().Store(&store_rgbpanel, &ConfigurationStore::rgb_panel);
}

void RgbPanelParams::Set()
{
#ifndef NDEBUG
    Dump();
#endif
}

void RgbPanelParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::RgbPanelParamsConst::kFileName);
    printf(" %s=%d\n", RgbPanelParamsConst::kCols.name, store_rgbpanel.cols);
    printf(" %s=%d\n", RgbPanelParamsConst::kRows.name, store_rgbpanel.rows);
    printf(" %s=%d\n", RgbPanelParamsConst::kChain.name, store_rgbpanel.chain);
    printf(" %s=%s [%d]]\n", RgbPanelParamsConst::kType.name, rgbpanel::GetType(common::FromValue<rgbpanel::Types>(store_rgbpanel.type)) , store_rgbpanel.type);

}
} // namespace json