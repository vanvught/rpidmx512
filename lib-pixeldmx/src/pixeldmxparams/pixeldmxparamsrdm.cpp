/**
 * @file pixeldmxparamsrdm.cpp
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


#undef NDEBUG

#if defined(DEBUG_PIXELDMX)
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "pixeldmxparamsrdm.h"
#include "pixeldmxstore.h"
#include "pixeltype.h"
#include "common/utils/utils_enum.h"
#include "pixeldmxconfiguration.h"
#include "rdmdeviceresponder.h"
 #include "firmware/debug/debug_debug.h"

void PixelDmxParamsRdm::SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length, [[maybe_unused]] bool do_update)
{
    DEBUG_PRINTF("port_index=%u, length=%u", port_index, length);
    assert(port_index == 0);

    if (length < pixeldmx::paramsdmx::kDmxFootprint)
    {
        return;
    }

    /*
     * Slot 1: type
     * Slot 2: nCount;
     * Slot 3: nGroupingCount
     * Slot 4: nMap;
     * Slot 5: nTestPattern
     * Slot 6: Program;
     */

    assert(pixeldmx::paramsdmx::kDmxFootprint == 6);

    const auto kLastIndex = pixeldmx::paramsdmx::kDmxFootprint - 1U;

    if (data[kLastIndex] == 0x00)
    {
        data_ = 0x00;
    }
    else
    {
        if ((data[kLastIndex] == 0xFF) && (data_ == 0x00))
        {
            DEBUG_PUTS("Program");
            data_ = 0xFF;

            auto slot_data = data[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotsInfo::TYPE)];
            uint8_t undefined = common::ToValue(pixel::Type::UNDEFINED);
            const auto kType = slot_data < undefined ? slot_data : undefined;
            slot_data = data[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotsInfo::MAP)];
            undefined = common::ToValue(pixel::Map::UNDEFINED);
            const auto kMap = slot_data < undefined ? slot_data : undefined;
            const auto kCount = data[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotsInfo::COUNT)];
            const auto kGroupingCount = data[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotsInfo::GROUPING_COUNT)];

            auto& configuration = PixelDmxConfiguration::Get();
            configuration.SetType(common::FromValue<pixel::Type>(kType));
            configuration.SetMap(common::FromValue<pixel::Map>(kMap));
            configuration.SetCount(kCount);
            configuration.SetGroupingCount(kGroupingCount);
            configuration.Validate(1);

            char description[rdm::personality::DESCRIPTION_MAX_LENGTH];
			pixeldmx::paramsdmx::SetPersonalityDescription(description);
            auto* personality = RDMDeviceResponder::Get()->GetPersonality(RDM_ROOT_DEVICE, 1);
            personality->SetDescription(description);

            dmxled_store::SaveType(common::ToValue(configuration.GetType()));
            dmxled_store::SaveMap(common::ToValue(configuration.GetMap()));
            dmxled_store::SaveCount(static_cast<uint16_t>(configuration.GetCount()));
            dmxled_store::SaveGroupingCount(static_cast<uint16_t>(configuration.GetGroupingCount()));
            dmxled_store::SaveTestPattern(data[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotsInfo::TEST_PATTERN)]);
        }
    }

    if ((data[kLastIndex] == 0x00) || (data[kLastIndex] == 0xFF))
    {
        pixeldmx::paramsdmx::Display(data);
    }
}
