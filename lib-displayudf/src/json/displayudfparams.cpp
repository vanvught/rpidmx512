/**
 * @file displayudfparams.cpp
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

#ifdef DEBUG_DISPLAYUDFPARAMS
#undef NDEBUG
#endif

#include <cstdint>

#include "json/displayudfparams.h"
#include "json/displayudfparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "common/utils/utils_array.h"
#include "common/utils/utils_enum.h"
 #include "firmware/debug/debug_debug.h"
#include "displayudf.h"

using common::store::displayudf::Flags;

namespace json
{
DisplayUdfParams::DisplayUdfParams()
{
    ConfigStore::Instance().Copy(&store_displayudf, &ConfigurationStore::display_udf);
}

void DisplayUdfParams::SetIntensity(const char* val, uint32_t len)
{
    if (len > 3) return;
    store_displayudf.intensity = ParseValue<uint8_t>(val, len);
}

void DisplayUdfParams::SetSleepTimeout(const char* val, uint32_t len)
{
    if (len > 3) return;
    store_displayudf.sleep_timeout = ParseValue<uint8_t>(val, len);
}

void DisplayUdfParams::SetFlipVertically(const char* val, uint32_t len)
{
    if (len != 1) return;
    store_displayudf.flags = common::SetFlagValue(store_displayudf.flags, Flags::Flag::kFlipVertically, val[0] != '0');
}

void DisplayUdfParams::SetLabel(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    if (val_len > 1) return;
    
    DEBUG_PRINTF("%.*s ->%.*s", key_len, key, val_len, val);

    const uint32_t kHash = Fnv1a32Runtime(key, static_cast<uint32_t>(key_len));
    bool matched = false;
    size_t i = 0;
    size_t j = 0;

    for (i = 0; i < common::ArraySize(kDisplayUdfKeys); ++i)
    {
        if (kDisplayUdfKeys[i].type == json::Key::kSimple)
        {
            j++;
            continue;
        }
        if (kDisplayUdfKeys[i].GetHash() == kHash)
        {
            matched = true;
            break;
        }
    }

    if (matched)
    {
        const auto kIndex = i - j;
        store_displayudf.label_index[kIndex] = ParseValue<uint8_t>(val, val_len);
    }
}

void DisplayUdfParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kDisplayUdfKeys);
    ConfigStore::Instance().Store(&store_displayudf, &ConfigurationStore::display_udf);

#ifndef NDEBUG
    Dump();
#endif
}

void DisplayUdfParams::SetAndShow()
{
    auto& displayudf = *DisplayUdf::Get();

    displayudf.SetContrast(store_displayudf.intensity);
    displayudf.SetSleepTimeout(store_displayudf.sleep_timeout);
    displayudf.SetFlipVertically(common::IsFlagSet(store_displayudf.flags, Flags::Flag::kFlipVertically));

    for (uint8_t i = 0; i < common::ArraySize(DisplayUdfParamsConst::kLabels); ++i)
    {
        const auto kLabelIndex = store_displayudf.label_index[i];
        if (kLabelIndex != 0)
        {
            displayudf.Set(kLabelIndex, common::FromValue<displayudf::Labels>(i));
        }
        else
        {
            displayudf.Set(255, common::FromValue<displayudf::Labels>(i));
        }
    }

    displayudf.Show();

#ifndef NDEBUG
    Dump();
#endif
}

void DisplayUdfParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DisplayUdfParamsConst::kFileName);
    printf(" %s=%u\n", DisplayUdfParamsConst::kIntensity.name, store_displayudf.intensity);
    printf(" %s=%u\n", DisplayUdfParamsConst::kSleepTimeout.name, store_displayudf.sleep_timeout);
    printf(" %s=%u\n", DisplayUdfParamsConst::kFlipVertically.name, common::IsFlagSet(store_displayudf.flags, Flags::Flag::kFlipVertically));

    for (uint32_t i = 0; i < common::ArraySize(DisplayUdfParamsConst::kLabels); ++i)
    {
        printf(" %s=%u\n", DisplayUdfParamsConst::kLabels[i].name, store_displayudf.label_index[i]);
    }
}
} // namespace json
