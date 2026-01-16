/**
 * @file json_config_displayudf.cpp
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

#include "json/displayudfparams.h"
#include "json/json_helpers.h"
#include "json/displayudfparamsconst.h"
#include "configurationstore.h"
#include "displayudf.h"

using common::store::displayudf::Flags;

namespace json::config
{
uint32_t GetDisplayUdf(char* buffer, uint32_t length)
{
    auto& displayudf = *DisplayUdf::Get();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    doc[DisplayUdfParamsConst::kIntensity.name] = displayudf.GetContrast();
	    doc[DisplayUdfParamsConst::kSleepTimeout.name] = displayudf.GetSleepTimeout();
	    doc[DisplayUdfParamsConst::kFlipVertically.name] = displayudf.GetFlipVertically();
	
	    for (uint32_t i = 0; i < common::ArraySize(DisplayUdfParamsConst::kLabels); ++i)
	    {
#if !defined(RDM_RESPONDER)
	        if (i == 8) continue;
#endif
	        const auto kLabel = displayudf.GetLabel(i);
	        if (kLabel > common::ArraySize(DisplayUdfParamsConst::kLabels))
	        {
	            doc[DisplayUdfParamsConst::kLabels[i].name] = "";
	        }
	        else
	        {
	            doc[DisplayUdfParamsConst::kLabels[i].name] = static_cast<uint32_t>(kLabel);
	        }
	    }
    });
}

void SetDisplayUdf(const char* buffer, uint32_t buffer_size)
{
    ::json::DisplayUdfParams displayudf_params;
    displayudf_params.Store(buffer, buffer_size);
    displayudf_params.SetAndShow();
}
} // namespace json::config