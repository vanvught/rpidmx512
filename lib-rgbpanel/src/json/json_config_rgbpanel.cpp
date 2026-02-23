/**
 * @file json_config_rgbpanel.cpp
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

#include "json/rgbpanelparams.h"
#include "json/json_helpers.h"
#include "json/rgbpanelparamsconst.h"
#include "rgbpanel.h"

namespace json::config
{
uint32_t GetRgbPanel(char* buffer, uint32_t length)
{
    auto& rgb_panel = RgbPanel::Instance();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    doc[RgbPanelParamsConst::kCols.name] = rgb_panel.GetColumns();
	    doc[RgbPanelParamsConst::kRows.name] = rgb_panel.GetRows();
	    doc[RgbPanelParamsConst::kChain.name] = rgb_panel.GetChain();
	    doc[RgbPanelParamsConst::kType.name] = rgbpanel::GetType(rgb_panel.GetType());
    });
}

void SetRgbPanel(const char* buffer, uint32_t buffer_size)
{
    ::json::RgbPanelParams rgbpanel_params;
    rgbpanel_params.Store(buffer, buffer_size);
    rgbpanel_params.Set();
}
} // namespace json::config
