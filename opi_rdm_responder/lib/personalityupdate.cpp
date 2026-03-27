/**
 * @file personalityupdate.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "common/utils/utils_enum.h"
#include "pixeltype.h"
#include "rdmresponder.h"
#include "pixeldmxconfiguration.h"
#include "pixeltestpattern.h"
#include "pixelpatterns.h"
#include "displayudf.h"
#include "firmware/pixeldmx/show.h"
 #include "firmware/debug/debug_debug.h"

void RDMResponder::PersonalityUpdate(uint32_t personality)
{
    DEBUG_PRINTF("personality=%u", personality);

#if defined(CONFIG_RDM_MANUFACTURER_PIDS_SET)
    assert(personality != 0);
    assert((personality - 1U) < static_cast<uint32_t>(pixel::LedType::kUndefined));

    const auto kType = static_cast<uint8_t>(personality - 1);
    auto& configuration = PixelDmxConfiguration::Get();
    configuration.SetType(common::FromValue<pixel::LedType>(kType));
    configuration.Validate(1);

    dmxled_store::SaveType(kType);

    common::firmware::pixeldmx::Show(7);

    const auto kTestPattern = PixelTestPattern::Get()->GetPattern();

    if (kTestPattern == pixelpatterns::Pattern::kNone)
    {
        PixelOutputType::Get()->ApplyConfiguration();
    }
    else
    {
        DisplayUdf::Get()->ClearEndOfLine();
        DisplayUdf::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(kTestPattern), static_cast<uint32_t>(kTestPattern));
    }
#else
    common::firmware::pixeldmx::Show(7);

    if (personality == 1)
    {
        const auto kTestPattern = PixelTestPattern::Get()->GetPattern();

        if (kTestPattern == pixelpatterns::Pattern::kNone)
        {
        	PixelOutputType::Get()->ApplyConfiguration();
        }
        else
        {
            DisplayUdf::Get()->ClearEndOfLine();
            DisplayUdf::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(kTestPattern), static_cast<uint32_t>(kTestPattern));
        }
    }
    else if (personality == 2)
    {
        DisplayUdf::Get()->ClearLine(3);
        DisplayUdf::Get()->ClearEndOfLine();
        DisplayUdf::Get()->Write(4, "Config Mode");
        DisplayUdf::Get()->ClearLine(5);
    }
#endif
}
