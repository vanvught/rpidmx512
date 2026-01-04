/**
 * @file ltcdisplayparams.cpp
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

#include "json/ltcdisplayparams.h"
#include "common/utils/utils_enum.h"
#include "json/ltcdisplayparamsconst.h"
#include "json/dmxledparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "ltcdisplaymax7219.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "ltcdisplayrgb.h"
#include "display.h"

using common::store::ltc::display::Flags;

namespace json
{
LtcDisplayParams::LtcDisplayParams()
{
    ConfigStore::Instance().Copy(&store_ltcdisplay, &ConfigurationStore::ltc_display);
}

void LtcDisplayParams::SetOledIntensity(const char* val, uint32_t len)
{
    store_ltcdisplay.oled_intensity = ParseValue<uint8_t>(val, len);
}

void LtcDisplayParams::SetRotaryFullstep(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_ltcdisplay.flags = common::SetFlagValue(store_ltcdisplay.flags, Flags::Flag::kRotaryFullStep, val[0] != '0');
}

void LtcDisplayParams::SetMaX7219Type(const char* val, uint32_t len)
{
    store_ltcdisplay.max7219_type = common::ToValue(LtcDisplayMax7219::Get()->GetType(val, len));
}

void LtcDisplayParams::SetMaX7219Intensity(const char* val, uint32_t len)
{
    if (len > 3) return;
    store_ltcdisplay.max7219_intensity = ParseValue<uint8_t>(val, len);
}

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
void LtcDisplayParams::SetPixelType(const char* val, uint32_t len)
{
    if (len == 8)
    {
        if (memcmp(val, "7segment", 8) == 0)
        {
            store_ltcdisplay.ws28xx_type = common::ToValue(ltc::display::rgb::WS28xxType::SEGMENT);
            return;
        }
    }

    store_ltcdisplay.ws28xx_type = common::ToValue(ltc::display::rgb::WS28xxType::MATRIX);
}
#endif

void LtcDisplayParams::SetInfoMsg(const char* val, uint32_t len)
{
    if (len > common::store::ltc::display::kMaxInfoMessage)
    {
        return;
    }

    memcpy(store_ltcdisplay.info_message, val, len);
    for (; len < common::store::ltc::display::kMaxInfoMessage; len++)
    {
        store_ltcdisplay.info_message[len] = ' ';
    }
}

void LtcDisplayParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kLtcDisplayKeys);
    ConfigStore::Instance().Store(&store_ltcdisplay, &ConfigurationStore::ltc_display);

#ifndef NDEBUG
    Dump();
#endif
}

void LtcDisplayParams::Set()
{
    auto* display = Display::Get();
    assert(display != nullptr);

    auto* max7219 = LtcDisplayMax7219::Get();
    assert(max7219 != nullptr);

    display->SetContrast(store_ltcdisplay.oled_intensity);
    max7219->SetType(common::FromValue<ltc::display::max7219::Types>(store_ltcdisplay.max7219_type));
    max7219->SetIntensity(store_ltcdisplay.max7219_intensity);

#ifndef NDEBUG
    Dump();
#endif
}

void LtcDisplayParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::LtcDisplayParamsConst::kFileName);

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    printf(" %s=%s [%u]\n", LtcDisplayParamsConst::kPixelType,
           store_ltcdisplay.ws28xx_display_type == static_cast<uint8_t>(ltc::display::rgb::WS28xxType::SEGMENT) ? "7segment" : "matrix",
           store_ltcdisplay.ws28xx_display_type);
    printf(" %s=%s [%u]\n", DmxLedParamsConst::kType.name, pixel::GetType(static_cast<pixel::Type>(store_ltcdisplay.ws28xx_type)),
           static_cast<int>(store_ltcdisplay.ws28xx_type));
    printf(" %s=%s [%u]\n", DmxLedParamsConst::kMap.name, pixel::GetMap(static_cast<pixel::Map>(store_ltcdisplay.ws28xx_rgb_mapping)),
           static_cast<int>(store_ltcdisplay.ws28xx_rgb_mapping));
#endif

    //    printf(" %s=%d\n", LtcDisplayParamsConst::INTENSITY, store_ltcdisplay.display_rgb_intensity);
    //    printf(" %s=%d\n", LtcDisplayParamsConst::COLON_BLINK_MODE, store_ltcdisplay.display_rgb_colon_blink_mode);
    //
    //    for (uint32_t index = 0; index < static_cast<uint32_t>(ltc::display::rgb::ColourIndex::LAST); index++)
    //    {
    //        printf(" %s=%.6x\n", LtcDisplayParamsConst::COLOUR[index], store_ltcdisplay.display_rgb_colour[index]);
    //    }

    printf(" %s=%s [%u]\n", LtcDisplayParamsConst::kMax7219Type.name,
           store_ltcdisplay.max7219_type == static_cast<uint8_t>(ltc::display::max7219::Types::kSegment) ? "7segment" : "matrix",
           store_ltcdisplay.max7219_type);
    printf(" %s=%u\n", LtcDisplayParamsConst::kMax7219Intensity.name, store_ltcdisplay.max7219_intensity);
    printf(" %s=%u\n", LtcDisplayParamsConst::kOledIntensity.name, store_ltcdisplay.oled_intensity);
    printf(" %s=%u\n", LtcDisplayParamsConst::kRotaryFullstep, common::IsFlagSet(store_ltcdisplay.flags, Flags::Flag::kRotaryFullStep));
}
} // namespace json